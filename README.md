# winfsp-uninstaller
Executable for uninstalling WinFsp in a WiX Burn bundle.

## What does this repository solve?

The [WinFsp](https://winfps.dev) installer required up to version 1.12.22339 (inclusive) for an update to manually uninstall the old version first.
Using a WiX Burn bundle with the default bootstrapper, it is not possible to trigger this uninstallation because you cannot obtain the product code of the old, installed version just using burn builtin actions.
This repository contains a solution to this by provding a small executable detecting if WinFsp  <= 1.12.2339 is installed and if so, uninstalls it.

The executable shows a confirmation dialog before uninstallation, the actual uninstallation is silent.
To skip the dialog and ensure full silent execution, pass `-q` as an argument.
To adjust the window title, pass `-t <My custom window title>` as an argument.
To adjust the message in the dialog box, pass `-m <My custom message>` as an argument.
To directly exit the application and do nothing, pass `-s` as an argument. This flag is necessary to skip execution during uninstall or other bundle actions.

## Usage
```xml
<Chain>
    <ExePackage Cache="yes" PerMachine="yes" Vital="yes" Permanent="no"
      SourceFile="resources\winfsp-uninstaller.exe"
      DisplayName="Removing outdated WinFsp Driver"
      Description="Workaround to remove old winfsp"
      DetectCondition="false"
      InstallCondition="InstalledLegacyWinFspVersion &lt;&gt; v0.0.0.0"
      >
        <CommandLine Condition="WixBundleUILevel &lt;= 3" InstallArgument="-q" RepairArgument="-q" UninstallArgument="-s"/>
        <!-- XML allows line breaks in attributes, hence keep the line breaks -->
        <CommandLine Condition="WixBundleUILevel &gt; 3" InstallArgument="-t &quot;MyApp Installer&quot; -m &quot;MyApp requires a newer version of the WinFsp driver. The installer will now uninstall WinFsp, possibly reboot, and afterwards proceed with this installation.

Do you want to continue?&quot;" RepairArgument="-q" UninstallArgument="-s"/>
        <ExitCode Behavior="success" Value="0"/>
        <ExitCode Behavior="success" Value="1"/>
        <ExitCode Behavior="error" Value="2"/>
        <ExitCode Behavior="error" Value="3"/>
        <ExitCode Behavior="forceReboot" Value="4"/>
        <ExitCode Behavior="success" Value="5"/>
    </ExePackage>
    <!-- install new version of WinFsp here -->
    <MsiPackage
      SourceFile="resources\winfsp.msi"
      ...
    />
</Chain>
```
