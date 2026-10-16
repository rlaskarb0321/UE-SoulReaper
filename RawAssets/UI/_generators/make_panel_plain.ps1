Add-Type -AssemblyName System.Drawing

function C([int]$a, [int]$r, [int]$g, [int]$b) { return [System.Drawing.Color]::FromArgb($a, $r, $g, $b) }

function New-PlainPanelImage {
	param([int]$Base, [int]$Scale, [bool]$Filled, [string]$OutPath)

	$S = $Scale
	$W = $Base * $S
	$H = $Base * $S
	$bmp = [System.Drawing.Bitmap]::new($W, $H, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
	$g = [System.Drawing.Graphics]::FromImage($bmp)
	$g.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::AntiAlias
	$g.PixelOffsetMode = [System.Drawing.Drawing2D.PixelOffsetMode]::HighQuality
	$g.Clear([System.Drawing.Color]::Transparent)

	function Get-Chamfer([float]$inset, [float]$c) {
		$x0 = $inset; $y0 = $inset; $x1 = $W - $inset; $y1 = $H - $inset
		return [System.Drawing.PointF[]]@(
			[System.Drawing.PointF]::new($x0 + $c, $y0),
			[System.Drawing.PointF]::new($x1 - $c, $y0),
			[System.Drawing.PointF]::new($x1, $y0 + $c),
			[System.Drawing.PointF]::new($x1, $y1 - $c),
			[System.Drawing.PointF]::new($x1 - $c, $y1),
			[System.Drawing.PointF]::new($x0 + $c, $y1),
			[System.Drawing.PointF]::new($x0, $y1 - $c),
			[System.Drawing.PointF]::new($x0, $y0 + $c)
		)
	}

	$chamfer = 10.0 * $S

	# same palette as the bracket version: neutral gunmetal / steel
	$fillColor = C 236 12 15 19
	$outline   = C 255 4 6 9
	$steel     = C 255 139 150 163
	$steelDark = C 255 58 66 76
	$topLight  = C 40 255 255 255

	$outer = Get-Chamfer (0.5 * $S) $chamfer

	if ($Filled) {
		$g.FillPolygon([System.Drawing.SolidBrush]::new($fillColor), $outer)
	}

	$g.DrawPolygon([System.Drawing.Pen]::new($outline, 1.0 * $S), $outer)

	$main = Get-Chamfer (2.0 * $S) ($chamfer - 1.0 * $S)
	$steelPen = [System.Drawing.Pen]::new($steel, 2.0 * $S)
	$steelPen.LineJoin = [System.Drawing.Drawing2D.LineJoin]::Miter
	$g.DrawPolygon($steelPen, $main)

	$inner = Get-Chamfer (4.5 * $S) ($chamfer - 3.0 * $S)
	$g.DrawPolygon([System.Drawing.Pen]::new($steelDark, 1.0 * $S), $inner)

	# uniform top light line; both ends stay inside the 16px corner region so the middle stretches cleanly
	$g.DrawLine([System.Drawing.Pen]::new($topLight, 1.0 * $S), (15 * $S), (3.5 * $S), ($W - 15 * $S), (3.5 * $S))

	$g.Dispose()
	$bmp.Save($OutPath, [System.Drawing.Imaging.ImageFormat]::Png)
	$bmp.Dispose()
}

$outDir = Join-Path $env:USERPROFILE "Downloads\SoulReaper_UI"
New-Item -ItemType Directory -Force -Path $outDir | Out-Null

foreach ($filled in $true, $false) {
	$name = if ($filled) { "Border" } else { "Frame" }
	foreach ($s in 1, 2) {
		$suffix = if ($s -eq 1) { "" } else { "@2x" }
		$file = Join-Path $outDir ("T_UI_PanelPlain_{0}{1}.png" -f $name, $suffix)
		New-PlainPanelImage -Base 128 -Scale $s -Filled $filled -OutPath $file
	}
}

Get-ChildItem $outDir -Filter "T_UI_PanelPlain_*" | ForEach-Object {
	$img = [System.Drawing.Image]::FromFile($_.FullName)
	"{0}  {1}x{2}" -f $_.Name, $img.Width, $img.Height
	$img.Dispose()
}
