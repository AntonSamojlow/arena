Write-Host "formatting files - please check the git diff"
# collect src files
$files = Get-ChildItem -Path $PSScriptRoot\src -Include *.cpp, *.h, *.ixx -Recurse -File
# collect src files
$files += Get-ChildItem -Path $PSScriptRoot\test -Include *.cpp, *.h, *.ixx -Recurse -File
# run clang-format
$files | ForEach-Object {Write-Host "formatting: $_"; clang-format.exe -i $_}
