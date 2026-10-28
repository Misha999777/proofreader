$ErrorActionPreference = "Stop"

Stop-Process -n ProofReader -ea Ignore
while (Get-Process ProofReader -ea Ignore) { Start-Sleep -m 50 }

Copy-Item packaging\*, build\$args\ProofReader.exe build\layout -Recurse -Force
Add-AppxPackage -Register build\layout\AppxManifest.xml

Start-Process "shell:AppsFolder\$((Get-StartApps ProofReader).AppID)"
while (!(Get-Process ProofReader -ea Ignore)) { Start-Sleep -m 50 }
