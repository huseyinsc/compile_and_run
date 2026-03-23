# Check for command-line arguments
if ($args.Count -ge 1) 
{
    $fileName = $args[0]
} 
else 
{
    $fileName = Read-Host -Prompt "Enter the file name"
}

Add-Type -AssemblyName System.IO.FileSystem
$fileNameWithoutExt = [System.IO.Path]::GetFileNameWithoutExtension($fileName)

$link_options = Read-Host -Prompt "Enter link options for $fileName" 

# Try to get the value of the variable if it exists
try {
    $invoked_value = Invoke-Expression "$link_options"
    if ($invoked_value -ne "") {
        $link_options = $invoked_value
    }
} catch {
    # If Invoke-Expression fails, keep the user input as is
    $link_options = $link_options
}

# Get the current directory name
$currentDirName = Split-Path -Leaf (Get-Location)

# Construct the run command based on the current directory name
if ($currentDirName -eq "src") {
    $run_cmd = 'g++ "' + $fileName + '" -o "..\bin\' + $fileNameWithoutExt + '.exe" ' + $link_options + ' & "..\bin\' + $fileNameWithoutExt + '.exe"'
} else {
    $run_cmd = 'g++ "' + $fileName + '" -o "' + $fileNameWithoutExt + '.exe" ' + $link_options + ' & "' + $fileNameWithoutExt + '.exe"'
}

$terminal = "cmd"

# Output the run command for debugging purposes
Write-Output "Start-Process $terminal -ArgumentList '/k', '$run_cmd'"

Start-Process $terminal -ArgumentList "/k", $run_cmd