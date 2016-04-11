function diffuser_top(od, h, wall, res) {
  return difference(
    cylinder({r: od/2, h: h, fn:res}),
    translate([0, 0, wall], cylinder({r: od/2 - wall, h: h-wall, fn: res}))
  );
}

function diffuser_bottom(top_od, x, h, wall, bottom_thickness, print_slop, res) {
  id = top_od - wall*2-print_slop;

  return difference(
      union(
        cylinder({r: id/2, h: bottom_thickness, fn: res}),
        translate([-x/2, -x/2, 0],
          cube({size: [x, x, h + bottom_thickness]})
          )
      ),
        translate([-x/2 + wall, -x/2 + wall, 0],
          cube({size: [x-wall*2, x-wall*2, h+wall + bottom_thickness]}))
    );
}


function assembly(diameter, height, cube_size, cube_h, wall, bottom_thickness, print_slop, res) {
  return rotate([180,0,0], union(
    diffuser_bottom(diameter, cube_size, cube_h, wall, bottom_thickness, print_slop, res),
    translate([0,0,-height+bottom_thickness], diffuser_top(diameter, height, wall, res))
    ));
}

function getParameterDefinitions() {
  return [
    { name: 'part', type: 'choice', initial: "assembly", values: ["top", "bottom", "assembly"], caption: "Part" },
    { name: 'diameter', type: 'float', initial: 30, caption: "Diffuser diameter" },
    { name: 'height', type: 'float', initial: 6, caption: "Diffuser height" },
    { name: 'cube_size', type: 'float', initial: 6-0.2, caption: "Cube size" },
    { name: 'cube_h', type: 'float', initial: 3, caption: "Cube height" },
    { name: 'bottom_thickness', type: 'float', initial: 1, caption: "Bottom thickness" },
    { name: 'wall', type: 'float', initial: 0.2, caption: "Wall thickness" },
    { name: 'print_slop', type: 'float', initial: 0.65, caption: "Print slop" },
    { name: 'res', type: 'int', initial: 120, caption: "Resolution" }
    ];
}

function main(params) {
  if (params.part == "top") {
    return diffuser_top(params.diameter, params.height, params.wall, params.res);
  } else if (params.part == "bottom") {
    return diffuser_bottom(params.diameter, params.cube_size, params.cube_h, params.wall, params.bottom_thickness, params.print_slop, params.res);
  } else if (params.part == "assembly") {
    return assembly(params.diameter, params.height, params.cube_size, params.cube_h, params.wall, params.bottom_thickness, params.print_slop, params.res);
  }
}
