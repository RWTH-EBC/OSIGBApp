$Title = "IOT Gateway flash tool"
$Info = "Which Gateway do you want to flash?"




$options = [System.Management.Automation.Host.ChoiceDescription[]] @("&Analog", "&Digital", "&Temperature", "&Quit")
[int]$defaultchoice = 3
$opt = $host.UI.PromptForChoice($Title , $Info , $Options, $defaultchoice)
switch($opt)
{
    0 { 
        Write-Host -ForegroundColor Yellow "Compile for Analog board"
        $target = 'aio'
    }
    1 { Write-Host -ForegroundColor Yellow "Compile for Digital board"
    $target = 'dio'
}
2 { Write-Host -ForegroundColor Yellow "Compile for Temperature board"
$target = 'temp'
}
3 { }
}

if ($target){
    C:\git\esp-idf\export.ps1
    # Write-Host -ForegroundColor Yellow "go to software.c"
    # Set-Location C:\git\iotdevelopment\software\software.c
    Write-Host -ForegroundColor Yellow "save original sdkconfig"
    
    if (Test-Path -Path ".\sdkconfig"){
        Rename-Item .\sdkconfig .\sdkconfig.orig
        $rename = 1
        }

    Write-Host "Ok. Preparing for $target !"
    Copy-Item .\sdkconfig.$target .\sdkconfig
    idf.py -B .\$target flash monitor 
    Remove-Item .\sdkconfig 
    Write-Host -ForegroundColor Yellow "finished $target skript."
    
    if ($rename) {
        Rename-Item .\sdkconfig.orig .\sdkconfig
    }
}
else {
    Write-Warning -Message "No work to do. Quit"
}
