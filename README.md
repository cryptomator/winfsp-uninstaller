# winfsp-uninstaller
Executable for uninstalling WinFsp in a WiX Burn bundle.

## What does this repostiroy solve?

The [WinFsp](https://winfps.dev) installer required up to version 1.12.22339 (inclusive) for an update to manually uninstall the old version first.
Using a WiX Burn bundle with the default bootstrapper, it is not possible to trigger this uninstallation because you cannot obtain the product code of the old, installed version just using burn builtin actions.
This repository contains a solution to this by provding a small executable detecting if WinFsp  <= 1.12.2339 is installed and if so, uninstalls it.

The executable shows a confirmation dialog before uninstallation, the actual uninstallation is silent.
To skip the dialog and ensure full silent execution, pass `-q` as the first argument.

## Usage
```xml
<Chain>
    <ExePackage Cache="yes" PerMachine="yes" Vital="yes" Permanent="no"
      SourceFile="resources\winfsp-uninstaller.exe"
      DisplayName="Removing outdated WinFsp Driver"
      Description="Executable to remove old winfsp"
      DetectCondition="false"
        <CommandLine Condition="WixBundleUILevel &lt;= 3" InstallArgument="-q" />
        <ExitCode Behavior="forceReboot" Value="0"/>
        <ExitCode Behavior="success" Value="1"/>
        <ExitCode Behavior="error" Value="2"/>
        <ExitCode Behavior="error" Value="3"/>
    </ExePackage>
    <!-- install new version of WinFsp here -->
    <MsiPackage
      SourceFile="resources\winfsp.msi"
      ...
    />
</Chain>
```
