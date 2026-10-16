Add-Type -AssemblyName System.Drawing

$dir = Join-Path $env:USERPROFILE "Downloads\SoulReaper_UI"
$script:bmp = [System.Drawing.Bitmap]::new(760, 850)
$script:g = [System.Drawing.Graphics]::FromImage($script:bmp)
$g = $script:g
$g.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::AntiAlias
$g.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
$g.TextRenderingHint = [System.Drawing.Text.TextRenderingHint]::AntiAliasGridFit
$g.PixelOffsetMode = [System.Drawing.Drawing2D.PixelOffsetMode]::HighQuality
$g.Clear([System.Drawing.Color]::FromArgb(255, 36, 42, 50))

function C([int]$a, [int]$r, [int]$gg, [int]$b) { return [System.Drawing.Color]::FromArgb($a, $r, $gg, $b) }
function RF([float]$x, [float]$y, [float]$w, [float]$h) { return [System.Drawing.RectangleF]::new($x, $y, $w, $h) }

$panelTex = [System.Drawing.Image]::FromFile((Join-Path $dir "T_UI_PanelPlain_Border@2x.png"))
$pouchTex = [System.Drawing.Image]::FromFile((Join-Path $dir "T_UI_Icon_GoldPouch@2x.png"))
$fadeTex  = [System.Drawing.Image]::FromFile((Join-Path $dir "T_UI_Divider_Fade@2x.png"))

function Draw-Panel([int]$x, [int]$y, [int]$w, [int]$h) {
	$m = 32; $d = 16
	$sw = $panelTex.Width; $sh = $panelTex.Height
	$xs = @(0, $m, ($sw - $m), $sw); $ys = @(0, $m, ($sh - $m), $sh)
	$dx = @($x, ($x + $d), ($x + $w - $d), ($x + $w)); $dy = @($y, ($y + $d), ($y + $h - $d), ($y + $h))
	for ($i = 0; $i -lt 3; $i++) { for ($j = 0; $j -lt 3; $j++) {
		$dst = [System.Drawing.Rectangle]::new($dx[$i], $dy[$j], ($dx[$i+1] - $dx[$i]), ($dy[$j+1] - $dy[$j]))
		$g.DrawImage($panelTex, $dst, $xs[$i], $ys[$j], ($xs[$i+1] - $xs[$i]), ($ys[$j+1] - $ys[$j]), [System.Drawing.GraphicsUnit]::Pixel)
	} }
}

function Text([string]$s, [float]$x, [float]$y, [float]$w, [float]$h, [float]$size, [bool]$bold, [System.Drawing.Color]$col, [string]$align = "Center", [string]$face = "Segoe UI") {
	$style = if ($bold) { [System.Drawing.FontStyle]::Bold } else { [System.Drawing.FontStyle]::Regular }
	$f = [System.Drawing.Font]::new($face, $size, $style, [System.Drawing.GraphicsUnit]::Pixel)
	$fmt = [System.Drawing.StringFormat]::new()
	$fmt.Alignment = [System.Drawing.StringAlignment]::$align
	$fmt.LineAlignment = [System.Drawing.StringAlignment]::Center
	$g.DrawString($s, $f, [System.Drawing.SolidBrush]::new($col), (RF $x $y $w $h), $fmt)
}

function Chamfer([float]$x, [float]$y, [float]$w, [float]$h, [float]$c) {
	return [System.Drawing.PointF[]]@(
		[System.Drawing.PointF]::new($x + $c, $y), [System.Drawing.PointF]::new($x + $w - $c, $y),
		[System.Drawing.PointF]::new($x + $w, $y + $c), [System.Drawing.PointF]::new($x + $w, $y + $h - $c),
		[System.Drawing.PointF]::new($x + $w - $c, $y + $h), [System.Drawing.PointF]::new($x + $c, $y + $h),
		[System.Drawing.PointF]::new($x, $y + $h - $c), [System.Drawing.PointF]::new($x, $y + $c))
}

function Draw-Icon([float]$cx, [float]$cy, [float]$r, [string]$kind) {
	$cols = @{
		Heal   = @((C 255 120 230 150), (C 255 36 130 70), "+")
		Revive = @((C 255 210 160 255), (C 255 100 50 170), [string][char]0x2665)
		Attack = @((C 255 255 150 130), (C 255 160 40 36), [string][char]0x25B2)
		Guard  = @((C 255 150 200 255), (C 255 40 90 160), [string][char]0x25C6)
		Elixir = @((C 255 255 226 140), (C 255 190 120 20), "*")
	}
	$c = $cols[$kind]
	$rect = RF ($cx - $r) ($cy - $r) (2 * $r) (2 * $r)
	$g.FillEllipse([System.Drawing.Drawing2D.LinearGradientBrush]::new($rect, $c[0], $c[1], 90.0), $rect)
	$g.DrawEllipse([System.Drawing.Pen]::new((C 255 20 14 8), 2.0), $rect)
	Text $c[2] ($cx - $r) ($cy - $r) (2 * $r) (2 * $r) ($r * 1.2) $true (C 255 255 255 255) "Center" "Segoe UI Symbol"
}

function Draw-Lock([float]$cx, [float]$cy) {
	$g.FillRectangle([System.Drawing.SolidBrush]::new((C 255 200 208 216)), ($cx - 9), ($cy - 2), 18, 14)
	$pen = [System.Drawing.Pen]::new((C 255 200 208 216), 3.0)
	$g.DrawArc($pen, ($cx - 6), ($cy - 12), 12, 14, 180, 180)
	$g.FillEllipse([System.Drawing.SolidBrush]::new((C 255 40 46 54)), ($cx - 2), ($cy + 2), 4, 4)
}

function Draw-Card([float]$x, [float]$y, [string]$kind, [string]$name, [int]$price, [int]$gold, [int]$owned, [bool]$locked) {
	$w = 108; $h = 101
	$pts = Chamfer $x $y $w $h 6
	$rect = RF $x $y $w $h
	$g.FillPolygon([System.Drawing.Drawing2D.LinearGradientBrush]::new($rect, (C 255 32 46 62), (C 255 14 20 28), 90.0), $pts)
	$border = if ($owned -gt 0) { C 255 94 143 119 } else { C 255 74 112 148 }
	$g.DrawPolygon([System.Drawing.Pen]::new($border, 2.0), $pts)
	Draw-Icon ($x + $w / 2) ($y + 32) 22 $kind
	Text $name $x ($y + 58) $w 16 13 $false (C 255 214 222 230)
	$affordable = ($price -le $gold)
	$pc = if ($affordable) { C 255 255 214 90 } else { C 255 227 106 98 }
	$g.FillEllipse([System.Drawing.SolidBrush]::new((C 255 240 190 60)), ($x + 30), ($y + 78), 12, 12)
	$g.DrawEllipse([System.Drawing.Pen]::new((C 255 122 78 12), 1.5), ($x + 30), ($y + 78), 12, 12)
	Text ("{0:N0}" -f $price) ($x + 44) ($y + 74) 56 20 14 $true $pc "Near"
	if ($owned -gt 0) {
		# "in bag" badge: check + count
		$bx = $x + $w - 40; $by = $y + 5
		$g.FillPolygon([System.Drawing.SolidBrush]::new((C 255 46 100 70)), (Chamfer $bx $by 36 18 4))
		Text ([string][char]0x2713 + " x" + $owned) $bx $by 36 18 11 $true (C 255 190 255 215)
	}
	if ($locked) {
		$g.FillPolygon([System.Drawing.SolidBrush]::new((C 170 8 10 14)), $pts)
		Draw-Lock ($x + $w / 2) ($y + 34)
	}
}

function Draw-Slot([float]$x, [float]$y, [int]$index, [string]$kind, [string]$name, [int]$count) {
	$w = 120; $h = 105
	$pts = Chamfer $x $y $w $h 6
	if ($kind -eq "") {
		$pen = [System.Drawing.Pen]::new((C 255 90 104 120), 2.0)
		$pen.DashStyle = [System.Drawing.Drawing2D.DashStyle]::Dash
		$g.FillPolygon([System.Drawing.SolidBrush]::new((C 255 14 18 24)), $pts)
		$g.DrawPolygon($pen, $pts)
		Text "+" $x ($y + 22) $w 40 40 $false (C 255 90 104 120)
		Text ("EMPTY  " + $index) $x ($y + 70) $w 18 12 $true (C 255 90 104 120)
	}
	else {
		$rect = RF $x $y $w $h
		$g.FillPolygon([System.Drawing.Drawing2D.LinearGradientBrush]::new($rect, (C 255 36 52 70), (C 255 16 22 30), 90.0), $pts)
		$g.DrawPolygon([System.Drawing.Pen]::new((C 255 74 112 148), 2.0), $pts)
		Draw-Icon ($x + $w / 2) ($y + 36) 24 $kind
		Text $name $x ($y + 66) $w 18 13 $false (C 255 214 222 230)
		Text ("x " + $count) $x ($y + 83) $w 18 14 $true (C 255 255 214 90)
		Text $index ($x + 6) ($y + 4) 18 16 11 $true (C 255 140 150 163) "Near"
	}
}

function Draw-Scene([int]$y0, [string]$tag, [int]$gold, [int]$bagCount, [bool]$full, [array]$bag, [array]$shop) {
	Text "ITEM SHOP" 30 $y0 200 28 22 $true (C 255 224 230 236) "Near"
	Text $tag 330 $y0 400 28 13 $false (C 255 140 150 163) "Far"

	# shop panel 700 x 125
	$sy = $y0 + 34
	Draw-Panel 30 $sy 700 125
	foreach ($ax in @(@(44, "L"), @(692, "R"))) {
		$p = $ax[0]; $glyph = if ($ax[1] -eq "L") { [string][char]0x25C0 } else { [string][char]0x25B6 }
		Text $glyph $p ($sy + 45) 24 34 18 $true (C 255 139 150 163) "Center" "Segoe UI Symbol"
	}
	$cx = 90
	foreach ($it in $shop) {
		Draw-Card $cx ($sy + 12) $it.kind $it.name $it.price $gold $it.owned $it.locked
		$cx += 118
	}
	if ($full) {
		$tx = 300; $ty = $sy - 2
		$g.FillPolygon([System.Drawing.SolidBrush]::new((C 235 70 46 12)), (Chamfer $tx $ty 300 24 5))
		$g.DrawPolygon([System.Drawing.Pen]::new((C 255 255 190 80), 2.0), (Chamfer $tx $ty 300 24 5))
		Text "Bag is full - up to 3 kinds" $tx $ty 300 24 13 $true (C 255 255 220 150)
	}

	# inventory panel 700 x 190 (125 items + 65 gold)
	$iy = $sy + 125 + 14
	Draw-Panel 30 $iy 700 190
	Text "BAG" 46 ($iy + 14) 110 18 12 $true (C 255 139 150 163) "Near"
	$counterCol = if ($full) { C 255 255 190 80 } else { C 255 190 255 215 }
	Text ("{0} / 3" -f $bagCount) 46 ($iy + 34) 110 40 32 $true $counterCol "Near"
	Text "Up to 3 kinds" 46 ($iy + 76) 130 18 12 $false (C 255 120 130 143) "Near"
	Text "per run" 46 ($iy + 92) 130 18 12 $false (C 255 120 130 143) "Near"
	$sx = 190
	for ($i = 0; $i -lt 3; $i++) {
		if ($i -lt $bag.Count) { Draw-Slot $sx ($iy + 10) ($i + 1) $bag[$i].kind $bag[$i].name $bag[$i].count }
		else { Draw-Slot $sx ($iy + 10) ($i + 1) "" "" 0 }
		$sx += 134
	}
	# divider + gold row
	$g.DrawImage($fadeTex, [System.Drawing.Rectangle]::new(46, ($iy + 125), 668, 5))
	$g.DrawImage($pouchTex, [System.Drawing.Rectangle]::new(50, ($iy + 134), 44, 44))
	Text "GOLD" 104 ($iy + 136) 80 16 12 $true (C 255 139 150 163) "Near"
	Text ("{0:N0}" -f $gold) 104 ($iy + 150) 200 30 26 $true (C 255 255 214 90) "Near"
	Text "Click shop item: buy   /   Click bag item: refund" 330 ($iy + 142) 380 30 12 $false (C 255 120 130 143) "Far"
}

$heal   = @{ kind = "Heal";   name = "Heal Potion";   price = 150;  owned = 2; locked = $false }
$revive = @{ kind = "Revive"; name = "Revive";        price = 400;  owned = 1; locked = $false }
$attack = @{ kind = "Attack"; name = "Attack Up";     price = 250;  owned = 0; locked = $false }
$guard  = @{ kind = "Guard";  name = "Guard Up";      price = 200;  owned = 0; locked = $false }
$elixir = @{ kind = "Elixir"; name = "Elixir";        price = 1500; owned = 0; locked = $false }

# scene A: 2 / 3
Draw-Scene 20 "Scene A  -  bag has 2 of 3 kinds" 1250 2 $false @(
	@{ kind = "Heal"; name = "Heal Potion"; count = 2 }, @{ kind = "Revive"; name = "Revive"; count = 1 }
) @($heal, $revive, $attack, $guard, $elixir)

# scene B: 3 / 3 (full) - shop items not in the bag are locked
$attack2 = @{ kind = "Attack"; name = "Attack Up"; price = 250; owned = 1; locked = $false }
$guard2  = @{ kind = "Guard";  name = "Guard Up";  price = 200; owned = 0; locked = $true }
$elixir2 = @{ kind = "Elixir"; name = "Elixir";    price = 1500; owned = 0; locked = $true }
Draw-Scene 430 "Scene B  -  bag is full (3 of 3)" 1000 3 $true @(
	@{ kind = "Heal"; name = "Heal Potion"; count = 2 }, @{ kind = "Revive"; name = "Revive"; count = 1 }, @{ kind = "Attack"; name = "Attack Up"; count = 1 }
) @($heal, $revive, $attack2, $guard2, $elixir2)

$out = Join-Path $dir "UI_ItemShop_Mockup.png"
$script:bmp.Save($out, [System.Drawing.Imaging.ImageFormat]::Png)
$g.Dispose(); $script:bmp.Dispose()
$out
