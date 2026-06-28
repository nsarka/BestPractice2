param(
  [string]$InputPath = (Join-Path $PSScriptRoot "..\note2.bmp"),
  [string]$OutputPath = (Join-Path $PSScriptRoot "..\BestPractice.ico")
)

Add-Type -AssemblyName System.Drawing

$source = [System.Drawing.Bitmap]::new((Resolve-Path $InputPath).Path)
$transparent = [System.Drawing.Bitmap]::new(
  $source.Width,
  $source.Height,
  [System.Drawing.Imaging.PixelFormat]::Format32bppArgb
)

$centerX = ($source.Width - 1) / 2.0
$centerY = ($source.Height - 1) / 2.0
$opaqueRadius = [Math]::Min($source.Width, $source.Height) * 0.466

for ($y = 0; $y -lt $source.Height; ++$y) {
  for ($x = 0; $x -lt $source.Width; ++$x) {
    $pixel = $source.GetPixel($x, $y)
    $distance = [Math]::Sqrt(
      [Math]::Pow($x - $centerX, 2) + [Math]::Pow($y - $centerY, 2)
    )

    if ($distance -le $opaqueRadius) {
      $transparent.SetPixel($x, $y, $pixel)
      continue
    }

    $alpha = [Math]::Max(0, [Math]::Min(255, 255 - [Math]::Min(
      $pixel.R, [Math]::Min($pixel.G, $pixel.B)
    )))
    if ($alpha -eq 0) {
      $transparent.SetPixel($x, $y, [System.Drawing.Color]::Transparent)
      continue
    }

    $recover = {
      param([int]$channel)
      [Math]::Max(0, [Math]::Min(255, 255 - (255 - $channel) * 255 / $alpha))
    }
    $transparent.SetPixel($x, $y, [System.Drawing.Color]::FromArgb(
      $alpha,
      (& $recover $pixel.R),
      (& $recover $pixel.G),
      (& $recover $pixel.B)
    ))
  }
}

$sizes = @(16, 24, 32, 48, 64, 128, 256)
$images = [System.Collections.Generic.List[byte[]]]::new()
foreach ($size in $sizes) {
  $bitmap = [System.Drawing.Bitmap]::new(
    $size,
    $size,
    [System.Drawing.Imaging.PixelFormat]::Format32bppArgb
  )
  $graphics = [System.Drawing.Graphics]::FromImage($bitmap)
  $graphics.Clear([System.Drawing.Color]::Transparent)
  $graphics.CompositingMode = [System.Drawing.Drawing2D.CompositingMode]::SourceCopy
  $graphics.CompositingQuality = [System.Drawing.Drawing2D.CompositingQuality]::HighQuality
  $graphics.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
  $graphics.PixelOffsetMode = [System.Drawing.Drawing2D.PixelOffsetMode]::HighQuality
  $graphics.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::HighQuality
  $graphics.DrawImage($transparent, 0, 0, $size, $size)
  $graphics.Dispose()

  $stream = [System.IO.MemoryStream]::new()
  $bitmap.Save($stream, [System.Drawing.Imaging.ImageFormat]::Png)
  $bitmap.Dispose()
  $images.Add($stream.ToArray())
  $stream.Dispose()
}

$outputDirectory = Split-Path -Parent $OutputPath
if ($outputDirectory -and !(Test-Path $outputDirectory)) {
  New-Item -ItemType Directory -Path $outputDirectory | Out-Null
}

$file = [System.IO.File]::Create($OutputPath)
$writer = [System.IO.BinaryWriter]::new($file)
$writer.Write([uint16]0)
$writer.Write([uint16]1)
$writer.Write([uint16]$sizes.Count)

$offset = 6 + 16 * $sizes.Count
for ($index = 0; $index -lt $sizes.Count; ++$index) {
  $size = $sizes[$index]
  $writer.Write([byte]$(if ($size -eq 256) { 0 } else { $size }))
  $writer.Write([byte]$(if ($size -eq 256) { 0 } else { $size }))
  $writer.Write([byte]0)
  $writer.Write([byte]0)
  $writer.Write([uint16]1)
  $writer.Write([uint16]32)
  $writer.Write([uint32]$images[$index].Length)
  $writer.Write([uint32]$offset)
  $offset += $images[$index].Length
}

foreach ($image in $images) {
  $writer.Write($image)
}

$writer.Dispose()
$file.Dispose()
$transparent.Dispose()
$source.Dispose()
