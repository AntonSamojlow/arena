Write-Host "formatting files - please check the git diff"
# src
$files = Get-ChildItem -Path .\src -Include *.cpp, *.h, *.ixx -Recurse -File
$files += Get-ChildItem -Path .\test -Include *.cpp, *.h, *.ixx -Recurse -File
$files | ForEach-Object {Write-Host "formatting: $_"; clang-format.exe -i $_}
