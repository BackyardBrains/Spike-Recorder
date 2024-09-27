@echo off
dotnet build
if errorlevel 1 (
    echo "Build  failed"
    exit /b 1
)

echo "MSI created successfully"