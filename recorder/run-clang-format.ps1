# Script to run clang-format on all source files within the ../src and ../test folders
# - use DryRun $true to output violations without changing the files
# - script exits if the expected clang-format version is not found (for consistent results between local and cloud)
param(
  [Parameter(HelpMessage = "Whether to report formatting violations without applying changes.")]
  [switch] $DryRun,
  [Parameter(HelpMessage = "The clang-format version to use. Fails if not found.")]
  [int] $Version = 16,
  [Parameter(HelpMessage = "Number of threads to use. Disabled for PS version < 7.")]
  [int] $ThrottleLimit = 16
)

# try find specified tool version
$clang_format_exe = (Get-Command "clang-format-$Version" -ErrorAction SilentlyContinue).Source
if (-Not $clang_format_exe) {
  $clang_format_exe = (Get-Command clang-format -ErrorAction SilentlyContinue).Source
  if (-Not $clang_format_exe) {
    Write-Error "Could not find clang-format with version '$Version'!"
    exit -1
  }
  $installed_version = & $clang_format_exe --version
  if (-Not ($installed_version -like "*version $Version*")) {
    Write-Error "Could not find clang-format with version '$Version'!"
    exit -1
  }
}
Write-Host "Found clang-format with version '$Version': $clang_format_exe"

# collect src files
$files = Get-ChildItem -Path $PSScriptRoot/src -Include *.cpp, *.h, *.ixx -Recurse -File
$files += Get-ChildItem -Path $PSScriptRoot/test -Include *.cpp, *.h, *.ixx -Recurse -File

if (0 -eq $files.Count) {
  Write-Error "No files to check"
  exit -1
}

# run clang-format
if ($DryRun) {
  $files | ForEach-Object { clang-format -i $_ --dry-run --Werror }
}
else {
  Write-Host "formatting files - please check the git diff"
  if ($parallel_enabled) {
    $files | ForEach-Object -Parallel {
      Write-Host "formatting: $_";
      & $using:clang_format_exe -i $_
    } `
    -ThrottleLimit $ThrottleLimit
  }
  else {
    $files | ForEach-Object { Write-Host "formatting: $_"; & $clang_format_exe -i $_ }
  }
}