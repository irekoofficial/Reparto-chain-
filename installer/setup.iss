[Setup]
AppName=Reparto Vocal Suite
AppVersion=1.0.0
AppPublisher=RepartoAudio
DefaultDirName={commonpf64}\Common Files\VST3
DisableDirPage=yes
DisableProgramGroupPage=yes
Uninstallable=yes
OutputDir=..\dist
OutputBaseFilename=RepartoVocalSuite-Setup
Compression=lzma
SolidCompression=yes
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequired=admin
UninstallFilesDir={commonappdata}\RepartoVocalSuite

[Files]
Source: "..\out\Reparto Vocal Suite.vst3\*"; DestDir: "{commonpf64}\Common Files\VST3\Reparto Vocal Suite.vst3"; Flags: recursesubdirs createallsubdirs ignoreversion
Source: "..\out\Reparto Vocal Suite.exe"; DestDir: "{commonpf64}\RepartoAudio\Reparto Vocal Suite"; Flags: ignoreversion skipifsourcedoesntexist

[Icons]
Name: "{autoprograms}\Reparto Vocal Suite (Standalone)"; Filename: "{commonpf64}\RepartoAudio\Reparto Vocal Suite\Reparto Vocal Suite.exe"; Check: FileExists(ExpandConstant('{commonpf64}\RepartoAudio\Reparto Vocal Suite\Reparto Vocal Suite.exe'))
