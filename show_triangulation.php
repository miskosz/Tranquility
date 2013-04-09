<?php
	header("Cache-Control: no-cache, must-revalidate");
	header("Expires: Mon, 26 Jul 1997 05:00:00 GMT");
?>

<!DOCTYPE html>
<head>
<title>Tranquility</title>
<meta charset="utf-8" />
</head>
<body>

<h2>Tranquility - Triagulations of Equilateral Triangles</h2>


<?php
	$palette = array(
		"#F7977A", "#F9AD81", "#FDC68A", "#FFF79A", "#C4DF9B", "#A2D39C",
		"#82CA9D", "#7BCDC8", "#6ECFF6", "#7EA7D8", "#8493CA", "#8882BE",
		"#A187BE", "#BC8DBF", "#F49AC2", "#F6989D");
	shuffle($palette);

	// Draws a triangle into svg canvas with triangular coordinates (x, y, z)
	// magnified by the factor of $zoom
	function draw_triangle($x, $y, $z, $zoom, $height) {
		$coords = array($zoom*$x, $zoom*$y, $zoom*$z);
		$base = array(
			array(-0.5,  0.866),
			array( 1.0,  0.0),
			array(-0.5, -0.866)
		);
		$origin = array(5.0, $height+5.0);
		$centroid = array(0.0, 0.0);
		
		for ($i = 0; $i < 3; $i++)
			for ($s = 0; $s < 2; $s++) {
				$triangle[$i][$s] =	$origin[$s] - $coords[$i]*$base[($i+1)%3][$s] + $coords[($i+1)%3]*$base[$i][$s];
				$centroid[$s] += $triangle[$i][$s]/3.0;
			}
				
		if (isset($_GET['fancy'])) {
			global $palette;
			$fill = $palette[abs($x+$y+$z) % count($palette)];
			$textfill = "#FFFFFF";
		} else {
			$fill = "#FFFFFF";
			$textfill = "#8F0000";
		}
		
		echo '<polygon points="'.$triangle[0][0].','.$triangle[0][1].' '.$triangle[1][0].','.$triangle[1][1].' '.$triangle[2][0].','.$triangle[2][1].'"'.
			'style="fill:'.$fill.';stroke:black;stroke-width:1;"/>';
		
		echo '<text style="font-size:'.((float)abs($x+$y+$z)*$zoom/3.0).'; fill:'.$textfill.'; text-anchor:middle; dominant-baseline:central;" 
			x="'.$centroid[0].'"  y="'.$centroid[1].'">'.abs($x+$y+$z).'</text>';
	}

	
	// Draws a triangulation with attributes:
	// $attributes['extended_info'] = true/false
	// $attributes['height'] = numeric
	function draw_triangulation($size, $nvertices, $faces_str, $coordinates, $attributes = array()) {

		// Set defaults
		if (!isset($attributes['height'])) $attributes['height'] = 500;
			
		// Get faces coordinates
		$coords = preg_split( "/( |,)/", $coordinates);
		$faces = preg_split( "/( |,)/", $faces_str);
		
		/*print_r($nvertices);
		print_r($size);
		print_r($coords);
		print_r($faces);*/
				
		// Draw the triangle
		echo '<svg id="svgelem" height="'.($attributes['height']+10).'" width="'.($attributes['height']/0.866+10).'" xmlns="http://www.w3.org/2000/svg">';	
		$zoom = ($attributes['height'] / 0.866) / $size;
		for ($i = 0; $i < $nvertices-2; $i++)
			draw_triangle(
				$coords[ord($faces[$i][0]) - ord('a')], 
				$coords[ord($faces[$i][1]) - ord('a')],
				$coords[ord($faces[$i][2]) - ord('a')],
				$zoom,
				$attributes['height']
			);
	
		echo '</svg><br />';

		// Print basic info
		echo "size = ".$size.", ";
		echo "triangles = ".($nvertices-2)."<br>";
		echo "<br>";
	}
	

	// Draws a triangulation with attributes:
	// $attributes['extended_info'] = true/false
	// $attributes['height'] = numeric
	function draw_triangulation_from_id($id_triangulation, $attributes = array()) {
		global $mysqli;
	
		// Set defaults
		if (!isset($attributes['extended_info'])) $attributes['extended_info'] = false;
		if (!isset($attributes['height'])) $attributes['height'] = 400;
	
		// Print triangulation id
		echo "<h3>id_triangulation = ".$id_triangulation."</h3>";
		if (!is_numeric($id_triangulation)) die("Parameter \"id_triangulation\" has to be numeric.");
			
		// Retrieve the data
		$result = mysqli_query($mysqli, "SELECT * FROM triangulation JOIN graph USING (id_graph) WHERE id_triangulation=".$id_triangulation);
		if (!$result) die('Failed to load the graph from database.');
		$row = mysqli_fetch_assoc($result);
		mysqli_free_result($result);
		if (!$row) die('Triangulation id '.$id_triangulation.' not found in the database.');
		
		// Print extended info
		if ($attributes['extended_info']) {
			echo "degenerate = ".$row['degenerated']."<br>";
			echo "vertex_color = ".$row['vertex_color']."<br>";
			echo "coordinates = ".$row['coordinates']."<br>";
			echo "<br>";
		}

		// Draw the triangulation
		$nvertices = (int)$row['nvertices'];
		if ($row['vertex_color'] == 'black') $faces = $row['white_faces'];
		else $faces = $row['black_faces'];
		draw_triangulation($row['size'], $nvertices, $faces, $row['coordinates'], $attributes);
	}

	////////////////////////////////////////////////////////////////////////////
	// Main                                                                   //
	////////////////////////////////////////////////////////////////////////////

	// Get triangulation id
	if (!isset($_GET['id_triangulation']) && !isset($_GET['id_graph']) && !isset($_POST['data_str'])) { ?>
		<p>Enter comma- or space- separated list of triangulation id's:</p>
		<form>
			<textarea type="textarea" name="id_triangulation" rows="5" cols="80"></textarea><br />
			<input type="submit" />
		</form>
		<p>OR enter graph id:</p>
		<form>
			<textarea type="textarea" name="id_graph" rows="5" cols="80"></textarea><br />
			<input type="submit" />
		</form>
		<p>OR paste in tranquility output:</p>
		<form method="post">
			<textarea type="textarea" name="data_str" rows="5" cols="80"></textarea><br />
			<input type="submit" />
		</form>

		<?php	
	}
	else {
	
		if 	(isset($_GET['id_triangulation']) || isset($_GET['id_graph'])) {
	
			// Connect to the database	
			$mysqli = new mysqli('localhost', 'postrelena', 'ahoj', 'postrelena');
			if (mysqli_connect_error())
				die('Connect Error ('.mysqli_connect_errno().') '.mysqli_connect_error());
	
			// Get triangulation id's
			if (isset($_GET['id_triangulation']))
				$id_triangulation = preg_split("/[\s,]+/", $_GET['id_triangulation']);
			else if (isset($_GET['id_graph'])) {
				$result = mysqli_query($mysqli, "SELECT id_triangulation FROM triangulation WHERE id_graph=".intval($_GET['id_graph']));
				if (!$result) die('Failed to load data from database.');
	
				$id_triangulation = array();
				while (($row = mysqli_fetch_assoc($result)))
					$id_triangulation[] = $row['id_triangulation'];
				mysqli_free_result($result);
			}
	
			// Draw triangulations
			foreach ($id_triangulation as $id)
				draw_triangulation_from_id($id, array("height"=>300));
		
			// Close the connection
			mysqli_close($mysqli);
		
		} else if (isset($_POST['data_str'])) {
			$data1 = explode("\n", $_POST['data_str']);
			foreach ($data1 as $data) {
				$triangulation_data = explode(" ", $data);
				draw_triangulation(
					$triangulation_data[0], // size
					$triangulation_data[1], // vertices
					$triangulation_data[2], // faces
					$triangulation_data[3]); // coordinates
			}
		}
	}	
?>

</body>
</html>
