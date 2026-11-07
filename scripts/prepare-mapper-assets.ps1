param(
    [string]$Root = "upstream/ScrcpyKeyMapper"
)

$ErrorActionPreference = 'Stop'
$rootPath = [System.IO.Path]::GetFullPath($Root)
$assets = @(
    @{ Path="vendor/bootstrap.min.css"; URL="https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/css/bootstrap.min.css"; Min=100000 },
    @{ Path="vendor/bootstrap.bundle.min.js"; URL="https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/js/bootstrap.bundle.min.js"; Min=60000 },
    @{ Path="vendor/bootstrap-icons.css"; URL="https://cdn.jsdelivr.net/npm/bootstrap-icons@1.11.3/font/bootstrap-icons.css"; Min=50000 },
    @{ Path="vendor/fonts/bootstrap-icons.woff2"; URL="https://cdn.jsdelivr.net/npm/bootstrap-icons@1.11.3/font/fonts/bootstrap-icons.woff2"; Min=10000 },
    @{ Path="vendor/fonts/bootstrap-icons.woff"; URL="https://cdn.jsdelivr.net/npm/bootstrap-icons@1.11.3/font/fonts/bootstrap-icons.woff"; Min=10000 },
    @{ Path="vendor/konva.min.js"; URL="https://unpkg.com/konva@9.2.3/konva.min.js"; Min=100000 },
    @{ Path="vendor/flag-icons/css/flag-icons.min.css"; URL="https://cdn.jsdelivr.net/gh/lipis/flag-icons@7.0.0/css/flag-icons.min.css"; Min=10000 },
    @{ Path="vendor/licenses/bootstrap.LICENSE"; URL="https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/LICENSE"; Min=500 },
    @{ Path="vendor/licenses/bootstrap-icons.LICENSE"; URL="https://cdn.jsdelivr.net/npm/bootstrap-icons@1.11.3/LICENSE"; Min=500 },
    @{ Path="vendor/licenses/konva.LICENSE"; URL="https://unpkg.com/konva@9.2.3/LICENSE"; Min=500 },
    @{ Path="vendor/licenses/flag-icons.LICENSE"; URL="https://cdn.jsdelivr.net/gh/lipis/flag-icons@7.0.0/LICENSE"; Min=500 }
)
foreach ($flag in @("us","br","sa","cn")) {
    foreach ($ratio in @("1x1","4x3")) {
        $assets += @{ Path="vendor/flag-icons/flags/$ratio/$flag.svg"; URL="https://cdn.jsdelivr.net/gh/lipis/flag-icons@7.0.0/flags/$ratio/$flag.svg"; Min=100 }
    }
}
foreach ($asset in $assets) {
    $target = Join-Path $rootPath $asset.Path
    New-Item -ItemType Directory -Force -Path (Split-Path $target) | Out-Null
    if (!(Test-Path $target) -or (Get-Item $target).Length -lt $asset.Min) {
        Invoke-WebRequest -Uri $asset.URL -OutFile $target -MaximumRedirection 5
    }
    if ((Get-Item $target).Length -lt $asset.Min) {
        throw "Recurso incompleto: $($asset.Path)"
    }
    Write-Host "$($asset.Path) [$((Get-Item $target).Length) bytes]"
}
Write-Host "Todos os recursos necessários ao editor foram preparados para uso offline."
