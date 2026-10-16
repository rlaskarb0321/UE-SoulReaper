Add-Type -AssemblyName System.Drawing

function C([int]$a, [int]$r, [int]$g, [int]$b) { return [System.Drawing.Color]::FromArgb($a, $r, $g, $b) }
function RF([float]$x, [float]$y, [float]$w, [float]$h) { return [System.Drawing.RectangleF]::new($x, $y, $w, $h) }

function New-PouchIcon {
	param([int]$Size, [string]$OutPath)

	$bmp = [System.Drawing.Bitmap]::new($Size, $Size, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
	$g = [System.Drawing.Graphics]::FromImage($bmp)
	$g.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::AntiAlias
	$g.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
	$g.TextRenderingHint = [System.Drawing.Text.TextRenderingHint]::AntiAliasGridFit
	$g.PixelOffsetMode = [System.Drawing.Drawing2D.PixelOffsetMode]::HighQuality
	$g.Clear([System.Drawing.Color]::Transparent)
	$k = $Size / 256.0
	$g.ScaleTransform($k, $k)

	$outline = C 255 30 19 10
	$outlinePen = [System.Drawing.Pen]::new($outline, 5.0)
	$outlinePen.LineJoin = [System.Drawing.Drawing2D.LineJoin]::Round

	# soft ground shadow
	$g.FillEllipse([System.Drawing.SolidBrush]::new((C 70 0 0 0)), (RF 44 218 168 20))

	# coins peeking out of the top (drawn first, the frill covers their lower half)
	foreach ($cp in @(@(108, 62), @(140, 54), @(168, 66))) {
		$cx = [float]$cp[0]; $cy = [float]$cp[1]
		$r = RF ($cx - 16) ($cy - 12) 32 24
		$cb = [System.Drawing.Drawing2D.LinearGradientBrush]::new($r, (C 255 255 226 140), (C 255 214 150 30), 90.0)
		$g.FillEllipse($cb, $r)
		$g.DrawEllipse([System.Drawing.Pen]::new((C 255 122 78 12), 3.0), $r)
		$g.DrawEllipse([System.Drawing.Pen]::new((C 150 160 104 16), 2.0), (RF ($cx - 9) ($cy - 6) 18 12))
	}

	# pouch body
	$body = [System.Drawing.Drawing2D.GraphicsPath]::new()
	$body.AddBezier([float]98, [float]96, [float]68, [float]112, [float]26, [float]150, [float]40, [float]196)
	$body.AddBezier([float]40, [float]196, [float]58, [float]238, [float]198, [float]238, [float]216, [float]196)
	$body.AddBezier([float]216, [float]196, [float]230, [float]150, [float]188, [float]112, [float]158, [float]96)
	$body.AddBezier([float]158, [float]96, [float]142, [float]104, [float]114, [float]104, [float]98, [float]96)
	$body.CloseFigure()
	$bodyBounds = $body.GetBounds()
	$bodyBrush = [System.Drawing.Drawing2D.LinearGradientBrush]::new($bodyBounds, (C 255 168 120 70), (C 255 78 50 28), 35.0)
	$g.FillPath($bodyBrush, $body)

	# body shading / highlight, clipped to the pouch
	$g.SetClip($body)
	$g.FillEllipse([System.Drawing.SolidBrush]::new((C 34 255 255 255)), (RF 40 118 56 92))
	$g.FillEllipse([System.Drawing.SolidBrush]::new((C 46 0 0 0)), (RF 150 120 110 130))
	$g.ResetClip()
	$g.DrawPath($outlinePen, $body)

	# gathered frill above the tie
	$frill = [System.Drawing.Drawing2D.GraphicsPath]::new()
	$frill.AddBezier([float]98, [float]98, [float]90, [float]84, [float]80, [float]74, [float]78, [float]58)
	$frill.AddBezier([float]78, [float]58, [float]108, [float]78, [float]148, [float]78, [float]178, [float]58)
	$frill.AddBezier([float]178, [float]58, [float]176, [float]74, [float]166, [float]84, [float]158, [float]98)
	$frill.CloseFigure()
	$frillBrush = [System.Drawing.Drawing2D.LinearGradientBrush]::new($frill.GetBounds(), (C 255 190 140 86), (C 255 112 74 42), 90.0)
	$g.FillPath($frillBrush, $frill)
	$g.DrawPath($outlinePen, $frill)

	# tie cord
	$cordOutline = [System.Drawing.Pen]::new($outline, 13.0)
	$cordOutline.StartCap = [System.Drawing.Drawing2D.LineCap]::Round
	$cordOutline.EndCap = [System.Drawing.Drawing2D.LineCap]::Round
	$cordGold = [System.Drawing.Pen]::new((C 255 240 200 90), 7.0)
	$cordGold.StartCap = [System.Drawing.Drawing2D.LineCap]::Round
	$cordGold.EndCap = [System.Drawing.Drawing2D.LineCap]::Round
	$g.DrawBezier($cordOutline, [float]92, [float]100, [float]112, [float]114, [float]144, [float]114, [float]164, [float]100)
	$g.DrawBezier($cordGold, [float]92, [float]100, [float]112, [float]114, [float]144, [float]114, [float]164, [float]100)

	# knot and tails
	foreach ($tail in @(@(152, 112, 156, 124, 172, 132, 168, 150), @(148, 112, 146, 128, 132, 136, 138, 152))) {
		$tp = [System.Drawing.Pen]::new($outline, 10.0); $tp.StartCap = [System.Drawing.Drawing2D.LineCap]::Round; $tp.EndCap = [System.Drawing.Drawing2D.LineCap]::Round
		$tg = [System.Drawing.Pen]::new((C 255 240 200 90), 5.0); $tg.StartCap = [System.Drawing.Drawing2D.LineCap]::Round; $tg.EndCap = [System.Drawing.Drawing2D.LineCap]::Round
		$g.DrawBezier($tp, [float]$tail[0], [float]$tail[1], [float]$tail[2], [float]$tail[3], [float]$tail[4], [float]$tail[5], [float]$tail[6], [float]$tail[7])
		$g.DrawBezier($tg, [float]$tail[0], [float]$tail[1], [float]$tail[2], [float]$tail[3], [float]$tail[4], [float]$tail[5], [float]$tail[6], [float]$tail[7])
	}
	$knot = RF 142 104 18 18
	$g.FillEllipse([System.Drawing.SolidBrush]::new((C 255 232 186 70)), $knot)
	$g.DrawEllipse([System.Drawing.Pen]::new($outline, 3.0), $knot)

	# gold coin emblem on the front
	$coin = RF 92 132 72 72
	$coinBrush = [System.Drawing.Drawing2D.LinearGradientBrush]::new($coin, (C 255 255 230 150), (C 255 210 142 26), 45.0)
	$g.FillEllipse($coinBrush, $coin)
	$g.DrawEllipse([System.Drawing.Pen]::new($outline, 4.0), $coin)
	$g.DrawEllipse([System.Drawing.Pen]::new((C 255 160 102 14), 3.0), (RF 100 140 56 56))

	$font = [System.Drawing.Font]::new("Arial", [float]40, [System.Drawing.FontStyle]::Bold, [System.Drawing.GraphicsUnit]::Pixel)
	$fmt = [System.Drawing.StringFormat]::new()
	$fmt.Alignment = [System.Drawing.StringAlignment]::Center
	$fmt.LineAlignment = [System.Drawing.StringAlignment]::Center
	$g.DrawString("G", $font, [System.Drawing.SolidBrush]::new((C 255 120 76 10)), (RF 92 134 72 72), $fmt)

	# shine on the coin
	$shinePen = [System.Drawing.Pen]::new((C 170 255 255 255), 3.0)
	$shinePen.StartCap = [System.Drawing.Drawing2D.LineCap]::Round
	$shinePen.EndCap = [System.Drawing.Drawing2D.LineCap]::Round
	$g.DrawArc($shinePen, (RF 104 144 48 48), [float]200, [float]55)

	$g.Dispose()
	$bmp.Save($OutPath, [System.Drawing.Imaging.ImageFormat]::Png)
	$bmp.Dispose()
}

$outDir = Join-Path $env:USERPROFILE "Downloads\SoulReaper_UI"
New-Item -ItemType Directory -Force -Path $outDir | Out-Null
New-PouchIcon -Size 256 -OutPath (Join-Path $outDir "T_UI_Icon_GoldPouch@2x.png")
New-PouchIcon -Size 128 -OutPath (Join-Path $outDir "T_UI_Icon_GoldPouch.png")

Get-ChildItem $outDir -Filter "T_UI_Icon_GoldPouch*" | ForEach-Object {
	$img = [System.Drawing.Image]::FromFile($_.FullName)
	"{0}  {1}x{2}" -f $_.Name, $img.Width, $img.Height
	$img.Dispose()
}
