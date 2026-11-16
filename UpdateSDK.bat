set /p "SDKPath=Enter the path to the SDK: "

tools\lighten_sdk.py --dump "%SDKPath%" --src CV2Merger --out CV2Merger --check