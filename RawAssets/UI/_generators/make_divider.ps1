Add-Type -AssemblyName System.Drawing

function C([int]$a, [int]$r, [int]$g, [int]$b) { return [System.Drawing.Color]::FromArgb($a, $r, $g, $b) }

# 3-slice style divider: pointed end caps, uniform middle (stretch horizontally only)
function New-CapDivider {
	param([int]$BaseW, [int]$BaseH, [int]$S, [string]$OutPath)
	$W = $BaseW * $S; $H = $BaseH * $S
	$bmp = [System.Drawing.Bitmap]::new($W, $H, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
	$g = [System.Drawing.Graphics]::FromImage($bmp)
	$g.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::AntiAlias
	$g.PixelOffsetMode = [System.Drawing.Drawing2D.PixelOffsetMode]::HighQuality
	$g.Clear([System.Drawing.Color]::Transparent)

	$ch = 3.0 * $S
	$half = $H / 2.0
	$pts = [System.Drawing.PointF[]]@(
		[System.Drawing.PointF]::new($ch, 0.5 * $S),
		[System.Drawing.PointF]::new($W - $ch, 0.5 * $S),
		[System.Drawing.PointF]::new($W - 0.5 * $S, $half),
		[System.Drawing.PointF]::new($W - $ch, $H - 0.5 * $S),
		[System.Drawing.PointF]::new($ch, $H - 0.5 * $S),
		[System.Drawing.PointF]::new(0.5 * $S, $half)
	)
	$rect = [System.Drawing.RectangleF]::new(0, 0, $W, $H)
	$body = [System.Drawing.Drawing2D.LinearGradientBrush]::new($rect, (C 255 156 166 178), (C 255 84 93 104), 90.0)
	$g.FillPolygon($body, $pts)

	$g.SetClip(([System.Drawing.Drawing2D.GraphicsPath]::new()))
	$g.ResetClip()

	# top highlight and bottom shade (uniform along the stretchable middle)
	$g.DrawLine([System.Drawing.Pen]::new((C 80 255 255 255), 1.0 * $S), ($ch + 1 * $S), (1.0 * $S), ($W - $ch - 1 * $S), (1.0 * $S))
	$g.DrawLine([System.Drawing.Pen]::new((C 120 0 0 0), 1.0 * $S), ($ch + 1 * $S), ($H - 1.0 * $S), ($W - $ch - 1 * $S), ($H - 1.0 * $S))

	# outline
	$g.DrawPolygon([System.Drawing.Pen]::new((C 200 4 6 9), 1.0 * $S), $pts)

	# end-cap ticks, all inside the 20px cap region
	$tick = [System.Drawing.Pen]::new((C 170 40 46 54), 1.0 * $S)
	foreach ($tx in 9, 12) {
		$g.DrawLine($tick, ($tx * $S), (1.5 * $S), ($tx * $S), ($H - 1.5 * $S))
		$g.DrawLine($tick, ($W - $tx * $S), (1.5 * $S), ($W - $tx * $S), ($H - 1.5 * $S))
	}

	$g.Dispose()
	$bmp.Save($OutPath, [System.Drawing.Imaging.ImageFormat]::Png)
	$bmp.Dispose()
}

# fading divider: alpha ramps to zero at both ends, can be stretched with Draw As: Image
function New-FadeDivider {
	param([int]$BaseW, [int]$BaseH, [int]$S, [string]$OutPath)
	$W = $BaseW * $S; $H = $BaseH * $S
	$bmp = [System.Drawing.Bitmap]::new($W, $H, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
	$g = [System.Drawing.Graphics]::FromImage($bmp)
	$g.Clear([System.Drawing.Color]::Transparent)

	function Fill-FadeRow([float]$y, [float]$h, [int]$r, [int]$gr, [int]$b, [int]$a) {
		$rect = [System.Drawing.RectangleF]::new(0, $y, $W, $h)
		$brush = [System.Drawing.Drawing2D.LinearGradientBrush]::new($rect, (C 0 $r $gr $b), (C 0 $r $gr $b), [System.Drawing.Drawing2D.LinearGradientMode]::Horizontal)
		$blend = [System.Drawing.Drawing2D.ColorBlend]::new(4)
		$blend.Colors = [System.Drawing.Color[]]@((C 0 $r $gr $b), (C $a $r $gr $b), (C $a $r $gr $b), (C 0 $r $gr $b))
		$blend.Positions = [float[]]@(0.0, 0.2, 0.8, 1.0)
		$brush.InterpolationColors = $blend
		$g.FillRectangle($brush, $rect)
	}

	Fill-FadeRow (0.0 * $S) (1.0 * $S) 255 255 255 70    # highlight
	Fill-FadeRow (1.0 * $S) (3.0 * $S) 139 150 163 255   # steel line
	Fill-FadeRow (4.0 * $S) (1.0 * $S) 0 0 0 130         # shade

	$g.Dispose()
	$bmp.Save($OutPath, [System.Drawing.Imaging.ImageFormat]::Png)
	$bmp.Dispose()
}

$outDir = Join-Path $env:USERPROFILE "Downloads\SoulReaper_UI"
New-Item -ItemType Directory -Force -Path $outDir | Out-Null

foreach ($s in 1, 2) {
	$suffix = if ($s -eq 1) { "" } else { "@2x" }
	New-CapDivider  -BaseW 128 -BaseH 6 -S $s -OutPath (Join-Path $outDir ("T_UI_Divider_Cap{0}.png" -f $suffix))
	New-FadeDivider -BaseW 256 -BaseH 5 -S $s -OutPath (Join-Path $outDir ("T_UI_Divider_Fade{0}.png" -f $suffix))
}

Get-ChildItem $outDir -Filter "T_UI_Divider_*" | ForEach-Object {
	$img = [System.Drawing.Image]::FromFile($_.FullName)
	"{0}  {1}x{2}" -f $_.Name, $img.Width, $img.Height
	$img.Dispose()
}
