Add-Type -AssemblyName System.Drawing

function C([int]$a, [int]$r, [int]$g, [int]$b) { return [System.Drawing.Color]::FromArgb($a, $r, $g, $b) }

function New-SlotImage {
	param(
		[int]$BaseW, [int]$BaseH, [int]$Scale,
		[System.Drawing.Color]$AccentTop, [System.Drawing.Color]$AccentBottom,
		[System.Drawing.Color]$BodyTop, [System.Drawing.Color]$BodyBottom,
		[System.Drawing.Color]$Border, [System.Drawing.Color]$Highlight,
		[bool]$ShowAccent, [string]$OutPath
	)

	$W = $BaseW * $Scale
	$H = $BaseH * $Scale
	$bmp = New-Object System.Drawing.Bitmap($W, $H, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
	$g = [System.Drawing.Graphics]::FromImage($bmp)
	$g.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::AntiAlias
	$g.PixelOffsetMode = [System.Drawing.Drawing2D.PixelOffsetMode]::HighQuality
	$g.Clear([System.Drawing.Color]::Transparent)

	function Get-Chamfer([float]$inset, [float]$chamfer) {
		$x0 = $inset; $y0 = $inset
		$x1 = $W - $inset; $y1 = $H - $inset
		$c = $chamfer
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

	$chamfer = 5.0 * $Scale
	$outer = Get-Chamfer (0.5 * $Scale) $chamfer
	$inner = Get-Chamfer (1.5 * $Scale) ($chamfer - 1.0 * $Scale)

	# body
	$rect = [System.Drawing.RectangleF]::new(0, 0, $W, $H)
	$bodyBrush = [System.Drawing.Drawing2D.LinearGradientBrush]::new($rect, $BodyTop, $BodyBottom, 90.0)
	$g.FillPolygon($bodyBrush, $outer)

	# clip to body for the interior details
	$path = New-Object System.Drawing.Drawing2D.GraphicsPath
	$path.AddPolygon($outer)
	$g.SetClip($path)

	# scanlines
	$scanPen = [System.Drawing.Pen]::new((C 22 0 0 0), 1.0 * $Scale)
	for ($y = 2 * $Scale; $y -lt $H; $y += 2 * $Scale) { $g.DrawLine($scanPen, 0, $y, $W, $y) }

	# bottom inner shade
	$shadeRect = [System.Drawing.RectangleF]::new(0, $H * 0.55, $W, $H * 0.45)
	$shadeBrush = [System.Drawing.Drawing2D.LinearGradientBrush]::new($shadeRect, (C 0 0 0 0), (C 70 0 0 0), 90.0)
	$g.FillRectangle($shadeBrush, $shadeRect)

	# left accent block
	if ($ShowAccent) {
		$accRect = [System.Drawing.RectangleF]::new(0, 0, 7 * $Scale, $H)
		$accBrush = [System.Drawing.Drawing2D.LinearGradientBrush]::new($accRect, $AccentTop, $AccentBottom, 90.0)
		$g.FillRectangle($accBrush, $accRect)
		$sepPen = [System.Drawing.Pen]::new((C 160 0 0 0), 1.0 * $Scale)
		$g.DrawLine($sepPen, 7 * $Scale, 0, 7 * $Scale, $H)
	}

	# right tick marks
	$tickPen = [System.Drawing.Pen]::new((C 150 $Border.R $Border.G $Border.B), 1.0 * $Scale)
	for ($i = 0; $i -lt 3; $i++) {
		$tx = $W - (14 + 4 * $i) * $Scale
		$g.DrawLine($tickPen, $tx, 9 * $Scale, $tx, $H - 9 * $Scale)
	}

	$g.ResetClip()

	# top highlight line
	$hiPen = [System.Drawing.Pen]::new($Highlight, 1.0 * $Scale)
	$g.DrawLine($hiPen, $chamfer + 2 * $Scale, 1.5 * $Scale, $W - $chamfer - 2 * $Scale, 1.5 * $Scale)

	# inner dark bevel and outer border
	$innerPen = [System.Drawing.Pen]::new((C 200 0 0 0), 1.0 * $Scale)
	$g.DrawPolygon($innerPen, $inner)
	$borderPen = [System.Drawing.Pen]::new($Border, 1.0 * $Scale)
	$g.DrawPolygon($borderPen, $outer)

	$g.Dispose()
	$bmp.Save($OutPath, [System.Drawing.Imaging.ImageFormat]::Png)
	$bmp.Dispose()
}

$outDir = Join-Path $env:USERPROFILE "Downloads\SoulReaper_UI"
New-Item -ItemType Directory -Force -Path $outDir | Out-Null

$variants = @(
	@{ Name = "Normal"; AT = (C 255 70 215 255); AB = (C 255 16 110 150); BT = (C 255 30 42 56); BB = (C 255 14 20 28); Bd = (C 255 74 112 148); Hi = (C 150 150 200 235); Acc = $true },
	@{ Name = "Host";   AT = (C 255 255 214 90); AB = (C 255 170 110 20); BT = (C 255 46 40 28); BB = (C 255 22 18 12); Bd = (C 255 176 142 70); Hi = (C 150 255 230 150); Acc = $true },
	@{ Name = "Empty";  AT = (C 255 90 100 110); AB = (C 255 50 58 66); BT = (C 255 24 30 38); BB = (C 255 14 18 24); Bd = (C 255 52 66 82); Hi = (C 60 120 140 160); Acc = $false }
)

foreach ($v in $variants) {
	foreach ($s in 1, 2) {
		$suffix = if ($s -eq 1) { "" } else { "@2x" }
		$file = Join-Path $outDir ("T_UI_LobbySlot_{0}{1}.png" -f $v.Name, $suffix)
		New-SlotImage -BaseW 700 -BaseH 35 -Scale $s -AccentTop $v.AT -AccentBottom $v.AB -BodyTop $v.BT -BodyBottom $v.BB -Border $v.Bd -Highlight $v.Hi -ShowAccent $v.Acc -OutPath $file
	}
}

Get-ChildItem $outDir | ForEach-Object {
	$img = [System.Drawing.Image]::FromFile($_.FullName)
	"{0}  {1}x{2}  {3} bytes" -f $_.Name, $img.Width, $img.Height, $_.Length
	$img.Dispose()
}
