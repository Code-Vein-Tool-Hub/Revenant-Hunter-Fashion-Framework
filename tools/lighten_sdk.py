#!/usr/bin/env python3
"""
lighten_sdk.py - prune a Dumper-7 C++ SDK dump down to what a mod actually uses.

Given a full Dumper-7 dump (the folder holding SDK.hpp, SDK/ and the support files) and the
mod's own source files, it works out which classes, structs, enums and functions the mod needs
and writes a much smaller SDK with the same layout (SDK.hpp, SDK/<Package>_*.hpp/.cpp, support
files), so the mod builds against it unchanged.

What gets kept:
  * every SDK type the mod's source names (SDK::FFoo, SDK::UBar::Baz, enum values, ...)
  * everything those types need to be complete: base classes and by-value members
  * for members the mod reads (World->OwningGameInstance, ...), the type they point at
  * for UFunctions the mod calls (matched by method name on kept classes): the function
    definition, its Params struct, and every type in its signature
  * all "Predefined" helpers on kept classes (UWorld::GetWorld, UEngine::GetEngine, ...)
  * whole packages passed with --keep-package (CoreUObject is always kept)
  * extra names from --keep / --keep-file, for anything the automatic scan misses

It also writes LightSDK_manifest.json next to SDK.hpp: the size, alignment and member offsets of
every kept type, every kept enum's values, the UFunctions the mod relies on, and the
build-specific Offsets from Basic.hpp. When you run it again on a new dump it compares against
that manifest and lists what changed. With --check it only compares and writes nothing, so after
a game update you can tell whether the mod's layouts changed or only the build-specific offsets.

Typical use:
  python lighten_sdk.py --dump D:\\Dumps\\CodeVein2\\CppSDK --src ..\\CV2Merger --out ..\\CV2Merger \\
                        --vcxproj ..\\CV2Merger\\CV2Merger.vcxproj
  python lighten_sdk.py --dump D:\\Dumps\\CodeVein2_new\\CppSDK --src ..\\CV2Merger --out ..\\CV2Merger --check
"""

from __future__ import annotations

import argparse
import json
import os
import re
import shutil
import sys
import time
from collections import deque
from dataclasses import dataclass, field
from pathlib import Path

SUPPORT_FILES = ["Assertions.inl", "NameCollisions.inl", "PropertyFixup.hpp", "UnrealContainers.hpp", "UtfN.hpp"]
VERBATIM_SDK_FILES = ["Basic.hpp", "Basic.cpp"]
ALWAYS_KEEP_PACKAGES = ["CoreUObject"]
MANIFEST_NAME = "LightSDK_manifest.json"
TOOL_VERSION = 1

# Templates whose argument is a UObject type that does not need to be complete.
WRAPPER_TEMPLATES = ("TSoftObjectPtr", "TSoftClassPtr", "TSubclassOf", "TWeakObjectPtr",
                     "TLazyObjectPtr", "TScriptInterface")

FILE_RE = re.compile(r"^(?P<pkg>.+)_(?P<kind>classes|structs|parameters)\.hpp$")
FUNC_FILE_RE = re.compile(r"^(?P<pkg>.+)_functions\.cpp$")
DECL_RE = re.compile(r"^(?P<kw>class|struct|union|enum class|enum)\s+(?:alignas\([^)]*\)\s+)?"
                     r"(?P<name>(?:\w+::)*\w+)(?:\s+final)?\s*(?::\s*(?P<base>.*))?$")
IDENT_RE = re.compile(r"(?<![\w:])((?:[A-Za-z_]\w*::)*[A-Za-z_]\w*)")
FIELD_RE = re.compile(r"^\t(?P<type>.+?)\s+(?P<name>\w+)(?:\[[^\]]*\])?(?:\s*:\s*\d+)?;\s*(?://.*)?$")
WRAPPER_BEFORE_RE = re.compile(r"(?:%s)<\s*(?:class|struct)?\s*$" % "|".join(WRAPPER_TEMPLATES))
ELABORATED_BEFORE_RE = re.compile(r"\b(?:class|struct|union)\s+$")
STRING_RE = re.compile(r'"(?:\\.|[^"\\\n])*"')
SIG_RE = re.compile(r"((?:\w+::)*\w+)::(~?\w+|operator\s*\S+?)\s*\(")
INCLUDE_RE = re.compile(r'^\s*#include\s+"(?:SDK/)?(?P<file>[^"]+)"')
ASSERT_SIZE_RE = re.compile(r"static_assert\(sizeof\((?P<t>[\w:]+)\) == (?P<v>0x[0-9A-Fa-f]+)")
ASSERT_ALIGN_RE = re.compile(r"static_assert\(alignof\((?P<t>[\w:]+)\) == (?P<v>0x[0-9A-Fa-f]+)")
ASSERT_OFFSET_RE = re.compile(r"static_assert\(offsetof\((?P<t>[\w:]+), (?P<m>\w+)\) == (?P<v>0x[0-9A-Fa-f]+)")
GETFUNCTION_RE = re.compile(r'GetFunction\("(?P<cls>[^"]*)", "(?P<fn>[^"]*)"\)')
OFFSETS_RE = re.compile(r"constexpr\s+int32\s+(?P<n>\w+)\s*=\s*(?P<v>0x[0-9A-Fa-f]+|\d+)")
DUMPER_ASSERT_BLOCK_RE = re.compile(r"^#define DUMPER7_ASSERTS_\S+ \\$")


# --------------------------------------------------------------------------------------------
# File I/O (lossless: keeps line endings and any odd bytes intact)
# --------------------------------------------------------------------------------------------

def read_lines(path: Path) -> list[str]:
    with open(path, "r", encoding="utf-8", errors="surrogateescape", newline="") as f:
        return f.read().splitlines(keepends=True)


def write_lines(path: Path, lines: list[str]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with open(path, "w", encoding="utf-8", errors="surrogateescape", newline="") as f:
        f.writelines(lines)


def code_part(line: str) -> str:
    """Line without its trailing // comment and with string literals blanked."""
    line = STRING_RE.sub('""', line)
    idx = line.find("//")
    return line if idx < 0 else line[:idx]


# --------------------------------------------------------------------------------------------
# Parsed dump model
# --------------------------------------------------------------------------------------------

@dataclass
class Block:
    """One top-level struct/class/union/enum declaration in a *_structs/_classes/_parameters file."""
    key: str                 # registry key ("FFoo", "PPABP_X::FFoo", "Params::UFoo_Bar")
    name: str                # name as written in the decl
    kw: str                  # class / struct / union / enum class / enum
    base: str | None
    file: str                # file name inside SDK/
    pre: list[str]           # comments / #pragma pack lines before the decl
    body: list[str]          # decl line ... "};"
    post: list[str]          # static_asserts / #pragma pack(pop) after "};"
    index: int               # position within its file
    _analysed: bool = False
    strong: set[str] = field(default_factory=set)               # need complete definitions
    fields: list[tuple[str, set[str]]] = field(default_factory=list)  # (field name, pointee/wrapped types)
    all_refs: set[str] = field(default_factory=set)             # every type mentioned (weak or strong)

    @property
    def is_enum(self) -> bool:
        return self.kw.startswith("enum")


@dataclass
class FuncDef:
    """One function definition in a *_functions.cpp file."""
    cls: str
    method: str
    file: str
    pre: list[str]
    body: list[str]          # signature line ... closing "}"
    index: int
    predefined: bool
    _deps: set[str] | None = None


@dataclass
class ParsedFile:
    name: str
    head: list[str]          # everything up to and including the namespace "{"
    tail: list[str]          # closing "}" and anything after
    blocks: list            # Block or FuncDef, in file order
    loose: list[str]         # unexpected top-level lines (kept verbatim, reported)


class Dump:
    def __init__(self, root: Path):
        self.root = root
        self.sdk_dir = root / "SDK"
        if not (root / "SDK.hpp").is_file() or not self.sdk_dir.is_dir():
            sys.exit(f"error: {root} does not look like a Dumper-7 dump (needs SDK.hpp and SDK/)")
        self.files: dict[str, ParsedFile] = {}
        self.types: dict[str, Block] = {}
        self.funcs_by_class: dict[str, list[FuncDef]] = {}
        self.package_of: dict[str, str] = {}
        self.duplicates: list[str] = []
        self.loose_count = 0

    # ---- parsing -------------------------------------------------------------------------

    def parse(self) -> None:
        names = sorted(os.listdir(self.sdk_dir))
        for n in names:
            m = FILE_RE.match(n)
            if m:
                self.package_of[n] = m.group("pkg")
                self._parse_header(n, params=(m.group("kind") == "parameters"))
                continue
            m = FUNC_FILE_RE.match(n)
            if m:
                self.package_of[n] = m.group("pkg")
                self._parse_functions(n)

    @staticmethod
    def _find_namespace(lines: list[str], ns: str) -> int:
        for i, l in enumerate(lines):
            if l.strip() == f"namespace {ns}" and i + 1 < len(lines) and lines[i + 1].strip() == "{":
                return i + 1
        return -1

    def _parse_header(self, fname: str, params: bool) -> None:
        lines = read_lines(self.sdk_dir / fname)
        ns = "SDK::Params" if params else "SDK"
        open_idx = self._find_namespace(lines, ns)
        if open_idx < 0:
            # Not a normal package file; keep it whole if anything includes it.
            self.files[fname] = ParsedFile(fname, lines, [], [], [])
            return
        head, blocks, loose, pending = lines[:open_idx + 1], [], [], []
        i, n = open_idx + 1, len(lines)
        tail: list[str] = []
        while i < n:
            raw = lines[i]
            s = raw.rstrip("\r\n")
            m = DECL_RE.match(s)
            if m and not s.rstrip().endswith(";"):
                pre = [l for l in pending if l.lstrip().startswith("//") or l.startswith("#pragma pack")]
                start = i
                while i < n and lines[i].rstrip("\r\n") != "};":
                    i += 1
                body = lines[start:i + 1]
                i += 1
                post_start = i
                while i < n and (lines[i].startswith("static_assert") or lines[i].startswith("#pragma pack(pop")
                                 or lines[i].startswith("DUMPER7_ASSERTS")):
                    i += 1
                post = lines[post_start:i]
                name = m.group("name")
                key = ("Params::" + name) if params else name
                kw = m.group("kw")
                base = m.group("base")
                if kw.startswith("enum"):
                    base = None
                elif base:
                    base = re.sub(r"^(public|protected|private)\s+", "", base.strip())
                b = Block(key, name, kw, base, fname, pre, body, post, len(blocks))
                if key in self.types:
                    self.duplicates.append(key)
                else:
                    self.types[key] = b
                blocks.append(b)
                pending = []
                continue
            if s == "}":
                tail = lines[i:]
                break
            if s.strip() and not s.lstrip().startswith("//") and not s.startswith("#pragma pack"):
                loose.append(raw)
                self.loose_count += 1
            pending.append(raw)
            i += 1
        self.files[fname] = ParsedFile(fname, head, tail, blocks, loose)

    def _parse_functions(self, fname: str) -> None:
        lines = read_lines(self.sdk_dir / fname)
        open_idx = self._find_namespace(lines, "SDK")
        if open_idx < 0:
            self.files[fname] = ParsedFile(fname, lines, [], [], [])
            return
        head, blocks, loose, pending = lines[:open_idx + 1], [], [], []
        i, n = open_idx + 1, len(lines)
        tail: list[str] = []
        while i < n:
            raw = lines[i]
            s = raw.rstrip("\r\n")
            if s == "}":
                tail = lines[i:]
                break
            if s and not s.startswith("//") and not s.startswith("#") and not s.startswith(" ") \
                    and not s.startswith("\t") and s != "{":
                start = i
                # signature may span lines until the "{" line
                while i < n and lines[i].rstrip("\r\n") != "{":
                    i += 1
                while i < n and lines[i].rstrip("\r\n") != "}":
                    i += 1
                body = lines[start:i + 1]
                i += 1
                sig = "".join(body[:2])
                m = SIG_RE.search(sig)
                pre = [l for l in pending if l.lstrip().startswith("//") or l.startswith("#")]
                predefined = any("Predefined Function" in l for l in pre)
                if m:
                    f = FuncDef(m.group(1), m.group(2), fname, pre, body, len(blocks), predefined)
                    self.funcs_by_class.setdefault(f.cls, []).append(f)
                    blocks.append(f)
                else:
                    loose.extend(pre + body)
                    self.loose_count += 1
                pending = []
                continue
            pending.append(raw)
            i += 1
        self.files[fname] = ParsedFile(fname, head, tail, blocks, loose)

    # ---- name resolution -----------------------------------------------------------------

    def resolve(self, tok: str) -> str | None:
        if tok.startswith("SDK::"):
            tok = tok[5:]
        if tok in self.types:
            return tok
        parts = tok.split("::")
        for k in range(len(parts) - 1, 0, -1):
            p = "::".join(parts[:k])
            if p in self.types:
                return p
        return None

    def _tokens(self, text: str):
        for m in IDENT_RE.finditer(text):
            r = self.resolve(m.group(1))
            if r:
                yield r, m.start(), m.end()

    # ---- dependency analysis -------------------------------------------------------------

    def analyse(self, b: Block) -> None:
        if b._analysed:
            return
        b._analysed = True
        if b.is_enum:
            return
        if b.base:
            for r, _, _ in self._tokens(b.base):
                b.strong.add(r)
                b.all_refs.add(r)
        is_class_like = b.kw in ("class", "struct", "union")
        for raw in b.body[1:-1]:
            line = raw.rstrip("\r\n")
            st = line.strip()
            if not st or st in ("{", "}", "public:", "private:", "protected:") or st.startswith("//"):
                continue
            fm = FIELD_RE.match(line) if is_class_like else None
            if fm and "(" not in fm.group("type"):
                ttext = code_part(fm.group("type"))
                weak: set[str] = set()
                for r, s0, e0 in self._tokens(ttext):
                    if r == b.key:
                        continue
                    b.all_refs.add(r)
                    after = ttext[e0:].lstrip()
                    if after.startswith("*") or WRAPPER_BEFORE_RE.search(ttext[:s0]):
                        weak.add(r)
                    else:
                        b.strong.add(r)
                b.fields.append((fm.group("name"), weak))
                continue
            code = code_part(line)
            is_decl = code.rstrip().endswith(";") and "(" in code
            for r, s0, _ in self._tokens(code):
                if r == b.key:
                    continue
                b.all_refs.add(r)
                if is_decl and ELABORATED_BEFORE_RE.search(code[:s0]):
                    continue  # elaborated type in a declaration: an incomplete type is fine
                b.strong.add(r)

    def func_deps(self, f: FuncDef) -> set[str]:
        if f._deps is None:
            deps: set[str] = set()
            for raw in f.body:
                for r, _, _ in self._tokens(code_part(raw)):
                    deps.add(r)
            f._deps = deps
        return f._deps


# --------------------------------------------------------------------------------------------
# Mod source scanning
# --------------------------------------------------------------------------------------------

COMMENT_RE = re.compile(r"//[^\n]*|/\*.*?\*/", re.S)


def scan_sources(paths: list[Path], exclude: set[str]) -> tuple[set[str], set[str], list[str]]:
    """Returns (identifiers, SDK-qualified names, SDK files included directly)."""
    idents: set[str] = set()
    qualified: set[str] = set()
    direct_includes: list[str] = []
    files: list[Path] = []
    for p in paths:
        if p.is_file():
            files.append(p)
        elif p.is_dir():
            for f in sorted(p.iterdir()):
                if f.is_file() and f.suffix.lower() in (".cpp", ".h", ".hpp", ".inl") and f.name not in exclude:
                    files.append(f)
    if not files:
        sys.exit("error: no mod source files found (check --src)")
    for f in files:
        text = f.read_text(encoding="utf-8", errors="replace")
        for line in text.splitlines():
            m = re.match(r'\s*#include\s+"SDK/([^"]+)"', line)
            if m:
                direct_includes.append(m.group(1))
        text = COMMENT_RE.sub(" ", text)
        text = STRING_RE.sub('""', text)
        idents.update(re.findall(r"[A-Za-z_]\w*", text))
        qualified.update(re.findall(r"SDK::((?:[A-Za-z_]\w*::)*[A-Za-z_]\w*)", text))
    return idents, qualified, direct_includes


# --------------------------------------------------------------------------------------------
# Closure
# --------------------------------------------------------------------------------------------

def compute_closure(dump: Dump, seeds: set[str], used_idents: set[str], keep_packages: set[str]):
    keep: set[str] = set()
    emit: set[int] = set()     # id() of FuncDefs to emit
    emit_funcs: list[FuncDef] = []
    queue: deque[str] = deque()

    def add(k: str):
        if k in dump.types and k not in keep:
            keep.add(k)
            queue.append(k)

    for f, pkg in dump.package_of.items():
        if pkg in keep_packages and f in dump.files:
            for b in dump.files[f].blocks:
                if isinstance(b, Block):
                    add(b.key)
    for s in seeds:
        add(s)

    while queue:
        k = queue.popleft()
        b = dump.types[k]
        dump.analyse(b)
        for d in b.strong:
            add(d)
        # members the mod reads: make what they point at complete too
        for fname, weak in b.fields:
            if fname in used_idents:
                for d in weak:
                    add(d)
        # function definitions of this class
        if not b.key.startswith("Params::"):
            for fn in dump.funcs_by_class.get(b.key, ()):
                if id(fn) in emit:
                    continue
                pkg = dump.package_of.get(fn.file)
                if fn.predefined or pkg in keep_packages or fn.method in used_idents:
                    emit.add(id(fn))
                    emit_funcs.append(fn)
                    for d in dump.func_deps(fn):
                        add(d)
        # Params structs: every type in an emitted function's signature must be complete
        if b.key.startswith("Params::"):
            for d in b.all_refs:
                add(d)

    # Safety net: every enum named anywhere in emitted text must be declared.
    enums = {k for k, v in dump.types.items() if v.is_enum}
    texts = [l for k in list(keep) for l in dump.types[k].body]
    texts += [l for fn in emit_funcs for l in fn.body]
    for l in texts:
        for m in IDENT_RE.finditer(code_part(l)):
            r = dump.resolve(m.group(1))
            if r in enums:
                keep.add(r)
    return keep, emit_funcs


# --------------------------------------------------------------------------------------------
# Output
# --------------------------------------------------------------------------------------------

def filter_includes(lines: list[str], emitted: set[str], drop_missing: bool = True) -> list[str]:
    out = []
    for l in lines:
        m = INCLUDE_RE.match(l)
        if m and drop_missing:
            target = m.group("file")
            base = os.path.basename(target)
            if (FILE_RE.match(base) or FUNC_FILE_RE.match(base)) and base not in emitted:
                continue
        out.append(l)
    return out


def build_output(dump: Dump, keep: set[str], emit_funcs: list[FuncDef], strip_asserts: bool):
    """Returns {relative path: lines} for everything in the light SDK."""
    nl = "\r\n"
    kept_by_file: dict[str, list] = {}
    for k in keep:
        b = dump.types[k]
        kept_by_file.setdefault(b.file, []).append(b)
    for fn in emit_funcs:
        kept_by_file.setdefault(fn.file, []).append(fn)
    for v in kept_by_file.values():
        v.sort(key=lambda x: x.index)

    emitted = set(kept_by_file) | set(VERBATIM_SDK_FILES)
    out: dict[str, list[str]] = {}
    for fname, items in kept_by_file.items():
        pf = dump.files[fname]
        lines = filter_includes(pf.head, emitted)
        for it in items:
            lines.append(nl)
            lines.extend(it.pre)
            lines.extend(it.body)
            if isinstance(it, Block):
                post = it.post
                if strip_asserts:
                    post = [l for l in post if not l.startswith("static_assert")]
                lines.extend(post)
            else:
                lines.append(nl)
        if pf.loose:
            lines.append(nl)
            lines.extend(pf.loose)
        lines.append(nl)
        lines.extend(pf.tail if pf.tail else ["}" + nl])
        out["SDK/" + fname] = lines

    for f in VERBATIM_SDK_FILES:
        out["SDK/" + f] = read_lines(dump.sdk_dir / f)

    sdk_hpp = filter_includes(read_lines(dump.root / "SDK.hpp"), emitted)
    out["SDK.hpp"] = sdk_hpp

    for f in SUPPORT_FILES:
        p = dump.root / f
        if not p.is_file():
            print(f"warning: support file {f} not found in dump, skipping")
            continue
        lines = read_lines(p)
        if f == "Assertions.inl":
            lines = filter_assertions(lines, dump, keep)
        out[f] = lines
    return out


def filter_assertions(lines: list[str], dump: Dump, keep: set[str]) -> list[str]:
    """Assertions.inl holds one DUMPER7_ASSERTS_<Type> macro per type; keep only kept types."""
    out, i, n = [], 0, len(lines)
    while i < n:
        if DUMPER_ASSERT_BLOCK_RE.match(lines[i].rstrip("\r\n")):
            j = i + 1
            while j < n and lines[j].strip():
                j += 1
            block = lines[i:j]
            m = re.search(r"(?:alignof|sizeof)\(([\w:]+)\)", "".join(block[1:3]))
            t = m.group(1) if m else None
            if t is None or t not in dump.types or t in keep or ("Params::" + t) in keep:
                out.extend(block)
                if j < n:
                    out.append(lines[j])
            i = j + 1
            continue
        out.append(lines[i])
        i += 1
    return out


# --------------------------------------------------------------------------------------------
# Manifest
# --------------------------------------------------------------------------------------------

def build_manifest(dump: Dump, keep: set[str], emit_funcs: list[FuncDef]) -> dict:
    types = {}
    for k in sorted(keep):
        b = dump.types[k]
        if b.is_enum:
            vals = [l.strip().rstrip(",") for l in b.body[2:-1] if "=" in l]
            types[k] = {"kind": "enum", "values": [re.sub(r"\s+", " ", v) for v in vals]}
            continue
        entry = {"kind": b.kw, "base": b.base, "size": None, "align": None, "members": {}}
        for l in b.post:
            if (m := ASSERT_SIZE_RE.search(l)):
                entry["size"] = m.group("v")
            elif (m := ASSERT_ALIGN_RE.search(l)):
                entry["align"] = m.group("v")
            elif (m := ASSERT_OFFSET_RE.search(l)):
                entry["members"][m.group("m")] = m.group("v")
        types[k] = entry
    funcs = {}
    for fn in sorted(emit_funcs, key=lambda f: (f.cls, f.method)):
        m = GETFUNCTION_RE.search("".join(fn.body))
        funcs[f"{fn.cls}::{fn.method}"] = (f"{m.group('cls')}.{m.group('fn')}" if m else
                                           ("predefined" if fn.predefined else "native"))
    offsets = {}
    basic = dump.sdk_dir / "Basic.hpp"
    in_offsets = False
    for l in read_lines(basic):
        if l.strip().startswith("namespace Offsets"):
            in_offsets = True
        elif in_offsets and l.strip() == "}":
            break
        elif in_offsets and (m := OFFSETS_RE.search(l)):
            offsets[m.group("n")] = m.group("v")
    version = ""
    for l in read_lines(dump.root / "SDK.hpp")[:20]:
        if re.match(r"^//\s*\d+\.\d+", l):
            version = l.strip("/ \r\n")
    return {"tool_version": TOOL_VERSION, "engine_version": version, "offsets": offsets,
            "types": types, "functions": funcs}


def diff_manifests(old: dict, new: dict) -> tuple[list[str], list[str]]:
    """Returns (breaking changes, informational changes)."""
    breaking, info = [], []
    for k, v in old.get("offsets", {}).items():
        nv = new["offsets"].get(k)
        if nv != v:
            info.append(f"Offsets::{k}: {v} -> {nv}  (build-specific; only matters if it isn't resolved at runtime)")
    if old.get("engine_version") != new.get("engine_version"):
        info.append(f"engine version: {old.get('engine_version')} -> {new.get('engine_version')}")
    ot, nt = old.get("types", {}), new.get("types", {})
    for k, ov in ot.items():
        nv = nt.get(k)
        if nv is None:
            breaking.append(f"{k}: no longer in the new dump (or no longer reached from the mod)")
            continue
        if ov.get("kind") == "enum":
            if ov.get("values") != nv.get("values"):
                breaking.append(f"{k}: enum values changed")
            continue
        for attr in ("size", "align", "base"):
            if ov.get(attr) != nv.get(attr):
                breaking.append(f"{k}: {attr} {ov.get(attr)} -> {nv.get(attr)}")
        om, nm = ov.get("members", {}), nv.get("members", {})
        for m, off in om.items():
            if m not in nm:
                breaking.append(f"{k}::{m}: member removed/renamed")
            elif nm[m] != off:
                breaking.append(f"{k}::{m}: offset {off} -> {nm[m]}")
        for m in nm:
            if m not in om:
                info.append(f"{k}::{m}: new member at {nm[m]}")
    for k in nt:
        if k not in ot:
            info.append(f"{k}: newly kept")
    of, nf = old.get("functions", {}), new.get("functions", {})
    for k, v in of.items():
        if k not in nf:
            breaking.append(f"function {k} ({v}): no longer in the new dump")
        elif nf[k] != v:
            breaking.append(f"function {k}: UFunction path {v} -> {nf[k]}")
    return breaking, info


# --------------------------------------------------------------------------------------------
# vcxproj sync
# --------------------------------------------------------------------------------------------

def sync_vcxproj(path: Path, out_files: dict[str, list[str]], out_root: Path) -> None:
    text = path.read_text(encoding="utf-8-sig")
    proj_dir = path.parent
    rel_prefix = os.path.relpath(out_root / "SDK", proj_dir).replace("/", "\\")
    cpp = sorted(p for p in out_files if p.startswith("SDK/") and p.endswith(".cpp"))
    wanted = [rel_prefix + "\\" + p[4:] for p in cpp]
    pat = re.compile(r'^[ \t]*<ClCompile Include="' + re.escape(rel_prefix) + r'\\[^"]+"\s*/>\r?\n', re.M)
    if re.search(r'<ClCompile Include="' + re.escape(rel_prefix) + r'\\[^"]+"\s*>', text):
        print("warning: vcxproj has SDK ClCompile items with metadata; not touching it. Add these by hand:")
        for w in wanted:
            print("   ", w)
        return
    matches = list(pat.finditer(text))
    if not matches:
        print("warning: no SDK ClCompile items found in the vcxproj; not touching it.")
        return
    indent = re.match(r"^[ \t]*", matches[0].group(0)).group(0)
    nl = "\r\n" if "\r\n" in text else "\n"
    new_items = "".join(f'{indent}<ClCompile Include="{w}" />{nl}' for w in wanted)
    first = matches[0].start()
    text = pat.sub("", text)
    text = text[:first] + new_items + text[first:]
    # drop ClInclude items pointing at SDK headers that no longer exist
    inc_pat = re.compile(r'^[ \t]*<ClInclude Include="' + re.escape(rel_prefix) + r'\\(?P<f>[^"]+)"\s*/>\r?\n', re.M)
    text = inc_pat.sub(lambda m: m.group(0) if ("SDK/" + m.group("f").replace("\\", "/")) in out_files else "", text)
    path.write_text(text, encoding="utf-8-sig")
    print(f"updated {path.name}: {len(wanted)} SDK source files")


# --------------------------------------------------------------------------------------------
# Main
# --------------------------------------------------------------------------------------------

def main() -> int:
    ap = argparse.ArgumentParser(description="Prune a Dumper-7 SDK dump down to what a mod uses.",
                                 formatter_class=argparse.RawDescriptionHelpFormatter, epilog=__doc__)
    ap.add_argument("--dump", required=True, type=Path, help="full dump folder (holds SDK.hpp and SDK/)")
    ap.add_argument("--src", required=True, type=Path, action="append",
                    help="mod source folder or file to scan (repeatable)")
    ap.add_argument("--out", required=True, type=Path,
                    help="where to write the light SDK (SDK.hpp, SDK/, support files go here)")
    ap.add_argument("--keep", action="append", default=[], metavar="NAME",
                    help="extra type to keep, or Type::Method to also keep a function (repeatable)")
    ap.add_argument("--keep-file", type=Path, help="file with one --keep name per line (# comments allowed)")
    ap.add_argument("--keep-package", action="append", default=[], metavar="PKG",
                    help="keep a whole package, e.g. Engine (repeatable)")
    ap.add_argument("--strip-asserts", action="store_true",
                    help="drop static_asserts from kept types (they only check the dump against itself)")
    ap.add_argument("--vcxproj", type=Path, help="update this project's SDK ClCompile list to match the output")
    ap.add_argument("--check", action="store_true",
                    help="only compare the dump against the existing manifest in --out; write nothing")
    ap.add_argument("--force", action="store_true",
                    help="allow replacing an SDK/ folder in --out that this tool didn't write")
    args = ap.parse_args()

    dump_root = args.dump.resolve()
    out_root = args.out.resolve()
    if out_root == dump_root:
        sys.exit("error: --out must differ from --dump (the output replaces SDK/ in --out)")

    t0 = time.time()
    print(f"parsing dump: {dump_root}")
    dump = Dump(dump_root)
    dump.parse()
    n_funcs = sum(len(v) for v in dump.funcs_by_class.values())
    print(f"  {len(dump.files)} files, {len(dump.types)} types, {n_funcs} function definitions "
          f"({time.time() - t0:.1f}s)")
    if dump.duplicates:
        print(f"  warning: {len(dump.duplicates)} duplicate type names (first kept): {dump.duplicates[:5]}")
    if dump.loose_count:
        print(f"  note: {dump.loose_count} unrecognised top-level lines kept verbatim in their files")

    exclude = set(SUPPORT_FILES) | {"SDK.hpp"}
    used_idents, qualified, direct_includes = scan_sources([p.resolve() for p in args.src], exclude)

    keep_names = list(args.keep)
    if args.keep_file:
        for l in args.keep_file.read_text(encoding="utf-8").splitlines():
            l = l.split("#", 1)[0].strip()
            if l:
                keep_names.append(l)

    seeds: set[str] = set()
    unresolved: list[str] = []
    for q in qualified:
        r = dump.resolve(q)
        if r:
            seeds.add(r)
    for tok in used_idents:
        if tok in dump.types:
            seeds.add(tok)
    for k in keep_names:
        r = dump.resolve(k)
        if r:
            seeds.add(r)
            parts = k.split("::")
            if len(parts) > 1:
                used_idents.add(parts[-1])
        else:
            unresolved.append(k)
    if unresolved:
        print(f"warning: --keep names not found in the dump: {', '.join(unresolved)}")

    keep_packages = set(ALWAYS_KEEP_PACKAGES) | set(args.keep_package)
    keep, emit_funcs = compute_closure(dump, seeds, used_idents, keep_packages)

    out_files = build_output(dump, keep, emit_funcs, args.strip_asserts)
    for inc in direct_includes:
        if "SDK/" + inc not in out_files:
            print(f"warning: mod includes SDK/{inc} directly, but nothing in it is used; it won't exist. "
                  f"Remove the include or --keep a type from it.")

    manifest = build_manifest(dump, keep, emit_funcs)
    old_manifest_path = out_root / MANIFEST_NAME
    old_manifest = None
    if old_manifest_path.is_file():
        try:
            old_manifest = json.loads(old_manifest_path.read_text(encoding="utf-8"))
        except (OSError, json.JSONDecodeError) as e:
            print(f"warning: couldn't read existing manifest: {e}")

    full_size = sum(f.stat().st_size for f in dump.sdk_dir.iterdir() if f.is_file())
    light_size = sum(len("".join(v).encode("utf-8", "surrogateescape")) for k, v in out_files.items()
                     if k.startswith("SDK/"))
    n_types = sum(1 for k in keep if not k.startswith("Params::"))
    n_params = len(keep) - n_types
    print(f"keeping {n_types} types, {n_params} Params structs, {len(emit_funcs)} functions "
          f"in {sum(1 for k in out_files if k.startswith('SDK/'))} files "
          f"({full_size / 1e6:.1f} MB -> {light_size / 1e6:.1f} MB)")

    if old_manifest:
        breaking, info = diff_manifests(old_manifest, manifest)
        print()
        if breaking:
            print(f"CHANGES THAT AFFECT THE MOD ({len(breaking)}):")
            for b in breaking:
                print("  !", b)
        else:
            print("no layout, enum or function changes in anything the mod uses.")
        if info:
            print(f"other changes ({len(info)}):")
            for i in info[:60]:
                print("   ", i)
            if len(info) > 60:
                print(f"    ... and {len(info) - 60} more")
        print()
    elif args.check:
        print(f"no existing {MANIFEST_NAME} in {out_root}; nothing to compare against")

    if args.check:
        return 1 if old_manifest and diff_manifests(old_manifest, manifest)[0] else 0

    # ---- write ------------------------------------------------------------------------------
    out_sdk = out_root / "SDK"
    if out_sdk.exists() and not old_manifest_path.is_file() and not args.force:
        sys.exit(f"error: {out_sdk} exists and wasn't written by this tool. Re-run with --force to replace it "
                 f"(make sure you still have the full dump somewhere else).")
    staging = out_root / ".lightsdk_staging"
    if staging.exists():
        shutil.rmtree(staging)
    for rel, lines in out_files.items():
        write_lines(staging / rel, lines)
    (staging / MANIFEST_NAME).write_text(json.dumps(manifest, indent=1), encoding="utf-8")

    if out_sdk.exists():
        shutil.rmtree(out_sdk)
    shutil.move(str(staging / "SDK"), str(out_sdk))
    for item in os.listdir(staging):
        dst = out_root / item
        if dst.exists():
            dst.unlink()
        shutil.move(str(staging / item), str(dst))
    shutil.rmtree(staging)
    print(f"wrote light SDK to {out_root} ({time.time() - t0:.1f}s)")

    if args.vcxproj:
        sync_vcxproj(args.vcxproj.resolve(), out_files, out_root)
    return 0


if __name__ == "__main__":
    sys.exit(main())
