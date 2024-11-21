; Installer Create Script 

#define MyAppName "MesConnector"
#define MyAppExeName "MesConnector.exe"
#define MyAppVersion "1.0"
#define MyAppPublisher "PolyWorks Shanghai Inc."
#define MyAppURL "https://www.polyworks.com.cn/"
#define MyFilePath "C:\Users\sanlozhang\Documents\GitHub\bosch-mes-connector\build\Desktop_Qt_6_8_0_MSVC2022_64bit-MinSizeRel\src\app"

[Setup]
; NOTE: The value of AppId uniquely identifies this application. Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{48B795E7-83D9-4A48-93BF-711FC25587AA}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
;AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={autopf64}\{#MyAppName}
DefaultGroupName={#MyAppName}
AllowNoIcons=yes
LicenseFile={#MyFilePath}\License.txt
; InfoBeforeFile={#MyFilePath}\License.txt
; Uncomment the following line to run in non administrative install mode (install for current user only.)
;PrivilegesRequired=lowest
PrivilegesRequiredOverridesAllowed=dialog
OutputDir={#SourcePath}
OutputBaseFilename=DataLoopToBoschMesConnector_V{#MyAppVersion}
SetupIconFile=C:\Users\sanlozhang\Documents\GitHub\bosch-mes-connector\src\server\img\Polyworks.ico
Compression=lzma
SolidCompression=yes
WizardStyle=modern

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "chinesesimplified"; MessagesFile: "compiler:Languages\ChineseSimplified.isl"

[Files]
Source: "{#MyFilePath}\{#MyAppExeName}"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#MyFilePath}\Qt6Core.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#MyFilePath}\Qt6Gui.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#MyFilePath}\Qt6Network.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#MyFilePath}\Qt6Svg.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#MyFilePath}\Qt6Widgets.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#MyFilePath}\imageformats\*"; DestDir: "{app}\imageformats\"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#MyFilePath}\networkinformation\*"; DestDir: "{app}\networkinformation\"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#MyFilePath}\platforms\*"; DestDir: "{app}\platforms\"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#MyFilePath}\styles\*"; DestDir: "{app}\styles\"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#MyFilePath}\tls\*"; DestDir: "{app}\tls\"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#MyFilePath}\mscl\*"; DestDir: "{app}\mscl\"; Flags: ignoreversion recursesubdirs createallsubdirs
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{group}\{cm:LaunchProgram,{#MyAppName}}"; Filename: "{app}\{#MyAppExeName}"
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{uninstallexe}"



