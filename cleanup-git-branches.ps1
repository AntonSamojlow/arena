param(
  [Parameter(HelpMessage = "Branch to keep - defaults to main")]
  [string] $Keep = "main"
)

git branch | ForEach-Object { if($_.Trim() -like "*$Keep")
{
  Write-Host "keeping $_"
} else
{
  Write-Host "deleting $_"
  git branch -d $_.Trim() -f };
}