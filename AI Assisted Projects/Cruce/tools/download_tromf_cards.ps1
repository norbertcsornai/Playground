param(
    [string]$SourceUrl = "https://tromf.ro/images/cards3.jpg",
    [string]$SourcePath = "assets/source/tromf_cards3.jpg",
    [string]$OutputDir = "assets/cards"
)

$ErrorActionPreference = "Stop"

New-Item -ItemType Directory -Force -Path (Split-Path $SourcePath) | Out-Null
New-Item -ItemType Directory -Force -Path $OutputDir | Out-Null

Invoke-WebRequest -UseBasicParsing $SourceUrl -OutFile $SourcePath

Add-Type -AssemblyName System.Drawing

$resolvedSourcePath = (Resolve-Path $SourcePath).Path
$source = [System.Drawing.Bitmap]::new($resolvedSourcePath)
$columns = 4
$rows = 9
$separator = 2
$cellWidth = [int]($source.Width / $columns)
$cellHeight = [int]($source.Height / $rows)

$suits = @(
    @{ Code = "r"; Name = "hearts"; Row = 0 },
    @{ Code = "d"; Name = "diamonds"; Row = 1 },
    @{ Code = "v"; Name = "clubs"; Row = 2 },
    @{ Code = "g"; Name = "spades"; Row = 3 }
)

$ranks = @(
    @{ Code = "A"; Name = "ace"; Column = 8 },
    @{ Code = "X"; Name = "ten"; Column = 7 },
    @{ Code = "4"; Name = "king"; Column = 2 },
    @{ Code = "3"; Name = "queen"; Column = 1 },
    @{ Code = "2"; Name = "jack"; Column = 0 },
    @{ Code = "9"; Name = "nine"; Column = 6 }
)

foreach ($suit in $suits) {
    foreach ($rank in $ranks) {
        $srcX = $suit.Row * $cellWidth
        $srcY = $rank.Column * $cellHeight

        $crop = [System.Drawing.Bitmap]::new($cellWidth, $cellHeight)
        $cropGraphics = [System.Drawing.Graphics]::FromImage($crop)
        $cropGraphics.DrawImage(
            $source,
            [System.Drawing.Rectangle]::new(0, 0, $cellWidth, $cellHeight),
            $srcX,
            $srcY,
            $cellWidth,
            $cellHeight,
            [System.Drawing.GraphicsUnit]::Pixel
        )

        $card = [System.Drawing.Bitmap]::new($cellWidth, ($cellHeight * 2) + $separator)
        $cardGraphics = [System.Drawing.Graphics]::FromImage($card)
        $cardGraphics.Clear([System.Drawing.Color]::White)
        $cardGraphics.DrawImage($crop, 0, 0)
        $cardGraphics.FillRectangle(
            [System.Drawing.Brushes]::Black,
            0,
            $cellHeight,
            $cellWidth,
            $separator
        )
        $crop.RotateFlip([System.Drawing.RotateFlipType]::Rotate180FlipNone)
        $cardGraphics.DrawImage($crop, 0, $cellHeight + $separator)

        $output = Join-Path $OutputDir "$($rank.Name)_$($suit.Name).png"
        $card.Save($output, [System.Drawing.Imaging.ImageFormat]::Png)

        $cardGraphics.Dispose()
        $card.Dispose()
        $cropGraphics.Dispose()
        $crop.Dispose()
    }
}

$source.Dispose()

@"
# Card Image Source

The card images in this directory were generated from the public sprite at:

https://tromf.ro/images/cards3.jpg

They were copied and cropped for this local Cruce project at the user's request.
"@ | Set-Content -Encoding UTF8 (Join-Path $OutputDir "SOURCE.md")
