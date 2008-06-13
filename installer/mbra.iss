#define BuildMode "opt"
#define AppName "MBRA"
#define AppVerName "MBRA 1.0"
#define Publisher "CHDS"
#define AppURL "http://svndmz.nps.edu"
#define AppExeName "mbra.exe"
#define QtDir "..\..\..\depend\qt"
#define VsDir "c:\Program Files\Microsoft Visual Studio 8"

[Setup]
AppId={{0D8A39DC-DE32-4A09-849E-BAC9D08A669A}
AppName={#AppName}
AppVerName={#AppVerName}
AppPublisher={#Publisher}
AppPublisherURL={#AppURL}
AppSupportURL={#AppURL}
AppUpdatesURL={#AppURL}
ShowLanguageDialog=no
DefaultDirName={pf}\{#AppName}
DefaultGroupName={#AppName}
UninstallDisplayName={#AppName}
UninstallDisplayIcon={app}\bin\{#AppExeName}
UninstallFilesDir={app}\uninst
OutputDir=.
OutputBaseFilename=mbrasetup
;OutputManifestFile=manifest.txt
WizardImageFile=WizImage.bmp
WizardSmallImageFile=WizSmallImage.bmp
SetupIconFile=Setup.ico
Compression=lzma
SolidCompression=yes

[Languages]
Name: english; MessagesFile: compiler:Default.isl

[Tasks]
Name: desktopicon; Description: {cm:CreateDesktopIcon}; GroupDescription: {cm:AdditionalIcons}; Languages:
Name: quicklaunchicon; Description: {cm:CreateQuickLaunchIcon}; GroupDescription: {cm:AdditionalIcons}; Flags: unchecked

[Icons]
Name: {group}\{#AppName}; Filename: {app}\bin\{#AppExeName}; WorkingDir: {app}; IconFilename: {app}\bin\{#AppExeName}; Tasks:
Name: {group}\{cm:UninstallProgram,{#AppName}}; Filename: {uninstallexe}
Name: {userdesktop}\{#AppName}; Filename: {app}\bin\{#AppExeName}; Tasks: desktopicon; WorkingDir: {app}; IconFilename: {app}\bin\{#AppExeName}; Languages:
Name: {userappdata}\Microsoft\Internet Explorer\Quick Launch\{#AppName}; Filename: {app}\bin\{#AppExeName}; Tasks: quicklaunchicon; WorkingDir: {app}; IconFilename: {app}\bin\{#AppExeName}; Languages:

[Run]
Filename: {app}\bin\{#AppExeName}; Description: {cm:LaunchProgram,{#AppName}}; Flags: nowait postinstall skipifsilent; Tasks: ; Languages:

[Registry]
Root: HKLM; Subkey: Software\DMZ\{#AppName}; ValueType: string; ValueName: workingDir; ValueData: {app}; Flags: uninsdeletekey
Root: HKCR; Subkey: .mbra; ValueType: string; ValueData: mbra.file; Flags: uninsdeletekey
Root: HKCR; Subkey: mbra.file\DefaultIcon; ValueType: string; ValueData: {app}\bin\mbra.exe,-110; Flags: uninsdeletekey
Root: HKCR; Subkey: mbra.file\shell\open\command; ValueType: string; ValueData: "{app}\bin\mbra.exe ""%1"""; Flags: uninsdeletekey

[Files]
Source: ..\..\..\bin\win32-{#BuildMode}\MBRA.app\*; DestDir: {app}; Flags: recursesubdirs
Source: {#QtDir}\bin\QtCore4.dll; DestDir: {app}\bin
Source: {#QtDir}\bin\QtGui4.dll; DestDir: {app}\bin
Source: {#QtDir}\bin\QtOpenGL4.dll; DestDir: {app}\bin
Source: {#QtDir}\bin\QtSvg4.dll; DestDir: {app}\bin
Source: {#QtDir}\bin\QtXml4.dll; DestDir: {app}\bin
Source: {#VsDir}\VC\redist\x86\Microsoft.VC80.CRT\*; DestDir: {app}\bin\Microsoft.VC80.CRT
