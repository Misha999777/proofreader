$ErrorActionPreference = "Stop"

Get-Process ProofReader -ErrorAction Ignore | Stop-Process
Start-Sleep -Milliseconds 1000

Copy-Item packaging\*, build\ProofReader.exe build\layout -Recurse -Force
Add-AppxPackage -Register build\layout\AppxManifest.xml

Start-Process "shell:AppsFolder\$((Get-AppxPackage MykhailoHroshevyi.ProofReader).PackageFamilyName)!App"
Start-Sleep -Milliseconds 1000
