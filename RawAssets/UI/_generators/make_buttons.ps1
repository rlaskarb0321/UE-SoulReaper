Add-Type -AssemblyName System.Drawing

function C([int]$a, [int]$r, [int]$g, [int]$b) { return [System.Drawing.Color]::FromArgb($a, $r, $g, $b) }

function Get-Palette([string]$Kind, [string]$State) {
	if ($State -eq "Disabled") {
		return @{
			BodyTop = (C 255 40 44 48); BodyBottom = (C 255 24 27 30)
			PlateTop = (C 255 58 63 68); PlateBottom = (C 255 38 42 46)
			Rim = (C 255 88 94 100); Accent = (C 0 0 0 0); AccentAlpha = 0; GlowAlpha = 0; Inv = $false
		}
	}
	if ($Kind -eq "Play") {
		switch ($State) {
			"Normal"  { return @{ BodyTop = (C 255 32 52 43); BodyBottom = (C 255 14 26 20); PlateTop = (C 255 54 86 70); PlateBottom = (C 255 28 50 40); Rim = (C 255 94 143 119); Accent = (C 255 98 227 155); AccentAlpha = 160; GlowAlpha = 0; Inv = $false } }
			"Hovered" { return @{ BodyTop = (C 255 38 62 51); BodyBottom = (C 255 16 30 23); PlateTop = (C 255 70 112 90); PlateBottom = (C 255 36 64 51); Rim = (C 255 143 227 182); Accent = (C 255 140 255 190); AccentAlpha = 255; GlowAlpha = 80; Inv = $false } }
			"Pressed" { return @{ BodyTop = (C 255 12 22 17); BodyBottom = (C 255 22 40 31); PlateTop = (C 255 24 44 35); PlateBottom = (C 255 40 70 56); Rim = (C 255 74 115 96); Accent = (C 255 70 170 115); AccentAlpha = 120; GlowAlpha = 0; Inv = $true } }
		}
	}
	else {
		switch ($State) {
			"Normal"  { return @{ BodyTop = (C 255 58 30 30); BodyBottom = (C 255 26 13 13); PlateTop = (C 255 96 52 52); PlateBottom = (C 255 52 26 26); Rim = (C 255 148 96 95); Accent = (C 255 227 106 98); AccentAlpha = 160; GlowAlpha = 0; Inv = $false } }
			"Hovered" { return @{ BodyTop = (C 255 70 36 36); BodyBottom = (C 255 32 16 16); PlateTop = (C 255 125 66 64); PlateBottom = (C 255 66 32 32); Rim = (C 255 233 149 144); Accent = (C 255 255 150 140); AccentAlpha = 255; GlowAlpha = 80; Inv = $false } }
			"Pressed" { return @{ BodyTop = (C 255 22 11 11); BodyBottom = (C 255 44 22 22); PlateTop = (C 255 46 24 24); PlateBottom = (C 255 76 40 40); Rim = (C 255 118 76 75); Accent = (C 255 180 84 78); AccentAlpha = 120; GlowAlpha = 0; Inv = $true } }
		}
	}
}

function Get-Poly([float]$W, [float]$H, [float]$Inset, [float]$Chamfer) {
	$c = [Math]::Max($Chamfer, 2.0)
	$x0 = $Inset; $y0 = $Inset; $x1 = $W - $Inset; $y1 = $H - $Inset
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

function New-ButtonImage {
	param([int]$BW, [int]$BH, [int]$S, [string]$Kind, [string]$State, [string]$OutPath)

	$W = $BW * $S; $H = $BH * $S
	$p = Get-Palette $Kind $State
	$bmp = [System.Drawing.Bitmap]::new($W, $H, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
	$g = [System.Drawing.Graphics]::FromImage($bmp)
	$g.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::AntiAlias
	$g.PixelOffsetMode = [System.Drawing.Drawing2D.PixelOffsetMode]::HighQuality
	$g.Clear([System.Drawing.Color]::Transparent)

	$c = 10.0 * $S
	$outer = Get-Poly $W $H (0.5 * $S) $c

	# body
	$bodyRect = [System.Drawing.RectangleF]::new(0, 0, $W, $H)
	$bodyBrush = [System.Drawing.Drawing2D.LinearGradientBrush]::new($bodyRect, $p.BodyTop, $p.BodyBottom, 90.0)
	$g.FillPolygon($bodyBrush, $outer)

	# faint scanlines inside the body
	$path = [System.Drawing.Drawing2D.GraphicsPath]::new()
	$path.AddPolygon($outer)
	$g.SetClip($path)
	$scanPen = [System.Drawing.Pen]::new((C 16 0 0 0), 1.0 * $S)
	for ($y = 2 * $S; $y -lt $H; $y += 2 * $S) { $g.DrawLine($scanPen, 0, $y, $W, $y) }
	$g.ResetClip()

	# dark outline + steel rim
	$g.DrawPolygon([System.Drawing.Pen]::new((C 255 4 6 9), 1.0 * $S), $outer)
	$rimPoly = Get-Poly $W $H (2.0 * $S) ($c - 1.5 * $S)
	$rimPen = [System.Drawing.Pen]::new($p.Rim, 2.0 * $S)
	$rimPen.LineJoin = [System.Drawing.Drawing2D.LineJoin]::Miter
	$g.DrawPolygon($rimPen, $rimPoly)

	# outer bevel (raised frame)
	$bevelPoly = Get-Poly $W $H (4.5 * $S) ($c - 4.0 * $S)
	Draw-Bevel $g $bevelPoly $p.Inv 90 130 (1.5 * $S)

	# raised plate
	$plateInset = 8.0 * $S
	$platePoly = Get-Poly $W $H $plateInset ($c - 7.0 * $S)
	$plateRect = [System.Drawing.RectangleF]::new(0, $plateInset, $W, ($H - 2 * $plateInset))
	$plateBrush = [System.Drawing.Drawing2D.LinearGradientBrush]::new($plateRect, $p.PlateTop, $p.PlateBottom, 90.0)
	$g.FillPolygon($plateBrush, $platePoly)
	Draw-Bevel $g $platePoly $p.Inv 110 150 (1.5 * $S)

	# accent line near the bottom of the plate
	if ($p.AccentAlpha -gt 0) {
		$acc = [System.Drawing.Color]::FromArgb($p.AccentAlpha, $p.Accent.R, $p.Accent.G, $p.Accent.B)
		$accPen = [System.Drawing.Pen]::new($acc, 2.0 * $S)
		$ay = $H - 13.0 * $S
		$g.DrawLine($accPen, (22 * $S), $ay, ($W - 22 * $S), $ay)
	}

	# hover glow just inside the rim
	if ($p.GlowAlpha -gt 0) {
		$glow = [System.Drawing.Color]::FromArgb($p.GlowAlpha, $p.Accent.R, $p.Accent.G, $p.Accent.B)
		$glowPen = [System.Drawing.Pen]::new($glow, 2.0 * $S)
		$glowPen.LineJoin = [System.Drawing.Drawing2D.LineJoin]::Miter
		$g.DrawPolygon($glowPen, (Get-Poly $W $H (5.5 * $S) ($c - 4.5 * $S)))
	}

	$g.Dispose()
	$bmp.Save($OutPath, [System.Drawing.Imaging.ImageFormat]::Png)
	$bmp.Dispose()
}

$outDir = Join-Path $env:USERPROFILE "Downloads\SoulReaper_UI"
New-Item -ItemType Directory -Force -Path $outDir | Out-Null

$buttons = @(
	@{ Kind = "Play";  W = 500; H = 100 },
	@{ Kind = "Leave"; W = 350; H = 80 }
)
$states = "Normal", "Hovered", "Pressed", "Disabled"

foreach ($b in $buttons) {
	foreach ($st in $states) {
		foreach ($s in 1, 2) {
			$suffix = if ($s -eq 1) { "" } else { "@2x" }
			$file = Join-Path $outDir ("T_UI_Btn_{0}_{1}{2}.png" -f $b.Kind, $st, $suffix)
			New-ButtonImage -BW $b.W -BH $b.H -S $s -Kind $b.Kind -State $st -OutPath $file
		}
	}
}

Get-ChildItem $outDir -Filter "T_UI_Btn_*" | ForEach-Object {
	$img = [System.Drawing.Image]::FromFile($_.FullName)
	"{0}  {1}x{2}" -f $_.Name, $img.Width, $img.Height
	$img.Dispose()
}
