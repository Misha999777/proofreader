$dir = "packaging" 
Copy-Item "build\ProofReader.exe" $dir

& makepri createconfig /cf $dir\pri.xml /dq en-US
& makepri new /pr $dir /cf $dir\pri.xml /of $dir\resources.pri
Remove-Item $dir\pri.xml

& makeappx pack /d $dir /p build\ProofReader.msix /o
Remove-Item $dir\resources.pri
Remove-Item $dir\ProofReader.exe
