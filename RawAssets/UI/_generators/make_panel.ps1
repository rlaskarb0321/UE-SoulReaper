Add-Type -AssemblyName System.Drawing

function C([int]$a, [int]$r, [int]$g, [int]$b) { return [System.Drawing.Color]::FromArgb($a, $r, $g, $b) }

function New-PanelImage {
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

	# palette: neutral gunmetal / steel, no cyan or gold so it never competes with the slot accents
	$fillColor    = C 236 12 15 19
	$outline      = C 255 4 6 9
	$steel        = C 255 139 150 163
	$steelDark    = C 255 58 66 76
	$bracket      = C 255 197 206 216
	$topLight     = C 40 255 255 255

	$outer = Get-Chamfer (0.5 * $S) $chamfer

	if ($Filled) {
		$fillBrush = [System.Drawing.SolidBrush]::new($fillColor)
		$g.FillPolygon($fillBrush, $outer)
	}

	# outermost dark outline
	$outlinePen = [System.Drawing.Pen]::new($outline, 1.0 * $S)
	$g.DrawPolygon($outlinePen, $outer)

	# main steel border
	$main = Get-Chamfer (2.0 * $S) ($chamfer - 1.0 * $S)
	$steelPen = [System.Drawing.Pen]::new($steel, 2.0 * $S)
	$steelPen.LineJoin = [System.Drawing.Drawing2D.LineJoin]::Miter
	$g.DrawPolygon($steelPen, $main)

	# inner dark steel line (double frame look)
	$inner = Get-Chamfer (4.5 * $S) ($chamfer - 3.0 * $S)
	$innerPen = [System.Drawing.Pen]::new($steelDark, 1.0 * $S)
	$g.DrawPolygon($innerPen, $inner)

	# uniform top light line (stretches cleanly along the top edge)
	$lightPen = [System.Drawing.Pen]::new($topLight, 1.0 * $S)
	$g.DrawLine($lightPen, 26 * $S, 3.5 * $S, $W - 26 * $S, 3.5 * $S)

	# corner brackets, all geometry stays inside the 24px corner region
	$brPen = [System.Drawing.Pen]::new($bracket, 3.0 * $S)
	$brPen.LineJoin = [System.Drawing.Drawing2D.LineJoin]::Miter
	$brPen.StartCap = [System.Drawing.Drawing2D.LineCap]::Flat
	$brPen.EndCap = [System.Drawing.Drawing2D.LineCap]::Flat
	$rel = @(@(7.0, 21.0), @(7.0, 13.0), @(13.0, 7.0), @(21.0, 7.0))
	foreach ($corner in @(@(1, 1), @(-1, 1), @(1, -1), @(-1, -1))) {
		$sx = $corner[0]; $sy = $corner[1]
		$pts = New-Object 'System.Collections.Generic.List[System.Drawing.PointF]'
		foreach ($p in $rel) {
			$x = if ($sx -gt 0) { $p[0] * $S } else { $W - $p[0] * $S }
			$y = if ($sy -gt 0) { $p[1] * $S } else { $H - $p[1] * $S }
			$pts.Add([System.Drawing.PointF]::new($x, $y))
		}
		$g.DrawLines($brPen, $pts.ToArray())
	}

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
		$file = Join-Path $outDir ("T_UI_Panel_{0}{1}.png" -f $name, $suffix)
		New-PanelImage -Base 128 -Scale $s -Filled $filled -OutPath $file
	}
}

Get-ChildItem $outDir -Filter "T_UI_Panel_*" | ForEach-Object {
	$img = [System.Drawing.Image]::FromFile($_.FullName)
	"{0}  {1}x{2}  {3} bytes" -f $_.Name, $img.Width, $img.Height, $_.Length
	$img.Dispose()
}
