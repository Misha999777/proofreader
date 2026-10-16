Get-Process ProofReader -ErrorAction Ignore | Stop-Process -PassThru | Wait-Process
Copy-Item packaging\*, build\ProofReader.exe build\layout -Recurse -Force

Add-AppxPackage -Register build\layout\AppxManifest.xml -ForceApplicationShutdown
Start-Process "shell:AppsFolder\$((Get-AppxPackage MykhailoHroshevyi.ProofReader).PackageFamilyName)!App"
