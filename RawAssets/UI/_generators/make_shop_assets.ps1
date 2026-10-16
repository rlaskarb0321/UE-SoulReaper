Add-Type -AssemblyName System.Drawing

function C([int]$a, [int]$r, [int]$g, [int]$b) { return [System.Drawing.Color]::FromArgb($a, $r, $g, $b) }
function RF([float]$x, [float]$y, [float]$w, [float]$h) { return [System.Drawing.RectangleF]::new($x, $y, $w, $h) }

function Get-Poly([float]$W, [float]$H, [float]$Inset, [float]$Chamfer) {
	$c = [Math]::Max($Chamfer, 1.5)
	$x0 = $Inset; $y0 = $Inset; $x1 = $W - $Inset; $y1 = $H - $Inset
	return [System.Drawing.PointF[]]@(
		[System.Drawing.PointF]::new($x0 + $c, $y0), [System.Drawing.PointF]::new($x1 - $c, $y0),
		[System.Drawing.PointF]::new($x1, $y0 + $c), [System.Drawing.PointF]::new($x1, $y1 - $c),
		[System.Drawing.PointF]::new($x1 - $c, $y1), [System.Drawing.PointF]::new($x0 + $c, $y1),
		[System.Drawing.PointF]::new($x0, $y1 - $c), [System.Drawing.PointF]::new($x0, $y0 + $c))
}

function Draw-Bevel($g, [System.Drawing.PointF[]]$P, [bool]$Inv, [int]$LightA, [int]$ShadeA, [float]$Width) {
	$lightPen = [System.Drawing.Pen]::new((C $LightA 255 255 255), $Width)
	$shadePen = [System.Drawing.Pen]::new((C $ShadeA 0 0 0), $Width)
	$lightPen.LineJoin = [System.Drawing.Drawing2D.LineJoin]::Miter
	$shadePen.LineJoin = [System.Drawing.Drawing2D.LineJoin]::Miter
	$tl = [System.Drawing.PointF[]]@($P[6], $P[7], $P[0], $P[1])
	$br = [System.Drawing.PointF[]]@($P[2], $P[3], $P[4], $P[5])
	if ($Inv) { $g.DrawLines($shadePen, $tl); $g.DrawLines($lightPen, $br) }
	else { $g.DrawLines($lightPen, $tl); $g.DrawLines($shadePen, $br) }
}

function New-Canvas([int]$W, [int]$H) {
	$bmp = [System.Drawing.Bitmap]::new($W, $H, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
	$g = [System.Drawing.Graphics]::FromImage($bmp)
	$g.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::AntiAlias
	$g.PixelOffsetMode = [System.Drawing.Drawing2D.PixelOffsetMode]::HighQuality
	$g.Clear([System.Drawing.Color]::Transparent)
	return @($bmp, $g)
}

# ---------------------------------------------------------------- item button 84 x 84
function New-ItemButton([int]$S, [string]$State, [string]$OutPath) {
	$B = 112; $W = $B * $S; $H = $B * $S
	$cv = New-Canvas $W $H; $bmp = $cv[0]; $g = $cv[1]
	$pal = switch ($State) {
		"Normal"   { @{ T = (C 255 36 52 70);  Bt = (C 255 16 23 32); Rim = (C 255 74 112 148);  Glow = 0;  Inv = $false } }
		"Hovered"  { @{ T = (C 255 46 66 88);  Bt = (C 255 20 30 42); Rim = (C 255 140 190 230); Glow = 90; Inv = $false } }
		"Pressed"  { @{ T = (C 255 14 20 28);  Bt = (C 255 26 38 52); Rim = (C 255 58 88 116);  Glow = 0;  Inv = $true } }
		"Disabled" { @{ T = (C 255 34 38 42);  Bt = (C 255 20 23 26); Rim = (C 255 70 76 82);   Glow = 0;  Inv = $false } }
		"InBag"    { @{ T = (C 255 30 56 46);  Bt = (C 255 14 28 22); Rim = (C 255 94 143 119); Glow = 0;  Inv = $false } }
	}
	$c = 8.0 * $S
	$outer = Get-Poly $W $H (0.5 * $S) $c
	$g.FillPolygon([System.Drawing.Drawing2D.LinearGradientBrush]::new((RF 0 0 $W $H), $pal.T, $pal.Bt, 90.0), $outer)

	$path = [System.Drawing.Drawing2D.GraphicsPath]::new(); $path.AddPolygon($outer); $g.SetClip($path)
	$scan = [System.Drawing.Pen]::new((C 14 0 0 0), 1.0 * $S)
	for ($y = 2 * $S; $y -lt $H; $y += 2 * $S) { $g.DrawLine($scan, 0, $y, $W, $y) }
	# price band at the bottom (22px): darker strip + separator
	$bandY = 84.0 * $S
	$g.FillRectangle([System.Drawing.SolidBrush]::new((C 90 0 0 0)), (RF 0 $bandY $W ($H - $bandY)))
	$g.DrawLine([System.Drawing.Pen]::new((C 120 $pal.Rim.R $pal.Rim.G $pal.Rim.B), 1.0 * $S), 0, $bandY, $W, $bandY)
	$g.ResetClip()

	$g.DrawPolygon([System.Drawing.Pen]::new((C 255 4 6 9), 1.0 * $S), $outer)
	$rimPen = [System.Drawing.Pen]::new($pal.Rim, 2.0 * $S); $rimPen.LineJoin = [System.Drawing.Drawing2D.LineJoin]::Miter
	$g.DrawPolygon($rimPen, (Get-Poly $W $H (2.0 * $S) ($c - 1.5 * $S)))
	Draw-Bevel $g (Get-Poly $W $H (4.0 * $S) ($c - 3.5 * $S)) $pal.Inv 80 120 (1.5 * $S)
	if ($pal.Glow -gt 0) {
		$gp = [System.Drawing.Pen]::new((C $pal.Glow 140 190 230), 2.0 * $S); $gp.LineJoin = [System.Drawing.Drawing2D.LineJoin]::Miter
		$g.DrawPolygon($gp, (Get-Poly $W $H (5.5 * $S) ($c - 4.5 * $S)))
	}
	$g.Dispose(); $bmp.Save($OutPath, [System.Drawing.Imaging.ImageFormat]::Png); $bmp.Dispose()
}

# ---------------------------------------------------------------- tray (9-slice, recessed)
function New-Tray([int]$S, [string]$OutPath) {
	$B = 64; $W = $B * $S; $H = $B * $S
	$cv = New-Canvas $W $H; $bmp = $cv[0]; $g = $cv[1]
	$c = 8.0 * $S
	$outer = Get-Poly $W $H (0.5 * $S) $c
	$g.FillPolygon([System.Drawing.SolidBrush]::new((C 235 8 10 13)), $outer)
	$g.DrawPolygon([System.Drawing.Pen]::new((C 255 4 6 9), 1.0 * $S), $outer)
	$rim = [System.Drawing.Pen]::new((C 255 58 66 76), 2.0 * $S); $rim.LineJoin = [System.Drawing.Drawing2D.LineJoin]::Miter
	$g.DrawPolygon($rim, (Get-Poly $W $H (2.0 * $S) ($c - 1.5 * $S)))
	# recessed bevel: shadow on top/left, light on bottom/right
	Draw-Bevel $g (Get-Poly $W $H (4.0 * $S) ($c - 3.5 * $S)) $true 50 170 (1.5 * $S)
	$g.Dispose(); $bmp.Save($OutPath, [System.Drawing.Imaging.ImageFormat]::Png); $bmp.Dispose()
}

# ---------------------------------------------------------------- arrow buttons 28 x 56
function New-Arrow([int]$S, [string]$Dir, [string]$State, [string]$OutPath) {
	$BW = 36; $BH = 64; $W = $BW * $S; $H = $BH * $S
	$cv = New-Canvas $W $H; $bmp = $cv[0]; $g = $cv[1]
	$pal = switch ($State) {
		"Normal"   { @{ T = (C 255 46 58 72); Bt = (C 255 24 30 38); Rim = (C 255 110 126 144); Glyph = (C 255 200 208 216); Inv = $false } }
		"Hovered"  { @{ T = (C 255 60 78 98); Bt = (C 255 30 40 52); Rim = (C 255 160 200 230); Glyph = (C 255 255 255 255); Inv = $false } }
		"Pressed"  { @{ T = (C 255 18 24 32); Bt = (C 255 32 42 54); Rim = (C 255 84 104 126); Glyph = (C 255 170 180 190); Inv = $true } }
		"Disabled" { @{ T = (C 255 34 38 42); Bt = (C 255 22 25 28); Rim = (C 255 64 70 76);   Glyph = (C 255 84 90 96);   Inv = $false } }
	}
	$c = 6.0 * $S
	$outer = Get-Poly $W $H (0.5 * $S) $c
	$g.FillPolygon([System.Drawing.Drawing2D.LinearGradientBrush]::new((RF 0 0 $W $H), $pal.T, $pal.Bt, 90.0), $outer)
	$g.DrawPolygon([System.Drawing.Pen]::new((C 255 4 6 9), 1.0 * $S), $outer)
	$rimPen = [System.Drawing.Pen]::new($pal.Rim, 1.5 * $S); $rimPen.LineJoin = [System.Drawing.Drawing2D.LineJoin]::Miter
	$g.DrawPolygon($rimPen, (Get-Poly $W $H (1.75 * $S) ($c - 1.2 * $S)))
	if ($State -ne "Disabled") { Draw-Bevel $g (Get-Poly $W $H (3.5 * $S) ($c - 3.0 * $S)) $pal.Inv 80 120 (1.0 * $S) }

	# triangle glyph
	$cx = $W / 2.0; $cy = $H / 2.0
	$off = if ($pal.Inv) { 1.0 * $S } else { 0.0 }
	if ($Dir -eq "Left") {
		$tri = [System.Drawing.PointF[]]@([System.Drawing.PointF]::new($cx + 5 * $S, $cy - 11 * $S + $off), [System.Drawing.PointF]::new($cx - 6 * $S, $cy + $off), [System.Drawing.PointF]::new($cx + 5 * $S, $cy + 11 * $S + $off))
	}
	else {
		$tri = [System.Drawing.PointF[]]@([System.Drawing.PointF]::new($cx - 5 * $S, $cy - 11 * $S + $off), [System.Drawing.PointF]::new($cx + 6 * $S, $cy + $off), [System.Drawing.PointF]::new($cx - 5 * $S, $cy + 11 * $S + $off))
	}
	$g.FillPolygon([System.Drawing.SolidBrush]::new($pal.Glyph), $tri)
	$g.Dispose(); $bmp.Save($OutPath, [System.Drawing.Imaging.ImageFormat]::Png); $bmp.Dispose()
}

$outDir = Join-Path $env:USERPROFILE "Downloads\SoulReaper_UI\Shop"
New-Item -ItemType Directory -Force -Path $outDir | Out-Null

foreach ($s in 2) {
	$sfx = if ($s -eq 1) { "" } else { "@2x" }
	foreach ($st in "Normal", "Hovered", "Pressed", "Disabled", "InBag") { New-ItemButton $s $st (Join-Path $outDir ("T_UI_ShopItem_{0}{1}.png" -f $st, $sfx)) }
	New-Tray $s (Join-Path $outDir ("T_UI_ShopTray{0}.png" -f $sfx))
	foreach ($d in "Left", "Right") { foreach ($st in "Normal", "Hovered", "Pressed", "Disabled") { New-Arrow $s $d $st (Join-Path $outDir ("T_UI_ArrowBtn_{0}_{1}{2}.png" -f $d, $st, $sfx)) } }
}

Get-ChildItem $outDir | Where-Object { $_.Name -like "T_UI_ShopItem_*" -or $_.Name -like "T_UI_ShopTray*" -or $_.Name -like "T_UI_ArrowBtn_*" } | ForEach-Object {
	$img = [System.Drawing.Image]::FromFile($_.FullName); "{0}  {1}x{2}" -f $_.Name, $img.Width, $img.Height; $img.Dispose()
} | Select-Object -First 40

