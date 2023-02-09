Write-Host "running clang tidy on all files - please be patient"
# collect src files
$files = Get-ChildItem -Path $PSScriptRoot\src -Include *.cpp, *.h, *.ixx -Recurse -File
# collect src files
$files += Get-ChildItem -Path $PSScriptRoot\test -Include *.cpp, *.h, *.ixx -Recurse -File
# run clang-format
$files | ForEach-Object {Write-Host "...clang-tidy checks: $_"; clang-tidy.exe $_}
Write-Host "...done running clang tidy on all files"
