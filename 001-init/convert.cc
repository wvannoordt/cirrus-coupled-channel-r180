#include "spade.h"
#include "PTL.h"

#include "io_control.h"

typedef double real_t;
typedef spade::ctrs::array<real_t, 3> v3d;
typedef spade::ctrs::array<int,    3> v3i;
typedef spade::ctrs::array<int,    4> v4i;
typedef spade::fluid_state::prim_t<real_t> prim_t;
typedef spade::fluid_state::flux_t<real_t> flux_t;
typedef spade::fluid_state::cons_t<real_t> cons_t;

int main(int argc, char** argv)
{
    spade::parallel::mpi_t group(&argc, &argv);
    
    local::io_control_t control(argc, argv);
    
    PTL::PropertyTree input;
    input.Read(control.get_input_file_name());
    
    const int blocks_x = input["Config"]["blocks_x"];
    const int blocks_y = input["Config"]["blocks_y"];
    const int blocks_z = input["Config"]["blocks_z"];
    
    const int cells_x  = input["Config"]["cells_x"];
    const int cells_y  = input["Config"]["cells_y"];
    const int cells_z  = input["Config"]["cells_z"];
    const int nexch    = input["Config"]["nexch"];
    
    spade::ctrs::array<int, 3> num_blocks(blocks_x, blocks_y, blocks_z);
    spade::ctrs::array<int, 3> cells_in_block(cells_x, cells_y, cells_z);
    spade::ctrs::array<int, 3> exchange_cells(nexch, nexch, nexch);

    spade::bound_box_t<real_t, 3> bounds;
    const real_t re_tau = 180.0;
    const real_t delta = 1.0;
    bounds.min(0) =  0.0;
    bounds.max(0) =  4.0*spade::consts::pi*delta;
    bounds.min(1) = -delta;
    bounds.max(1) =  delta;
    bounds.min(2) =  0.0;
    bounds.max(2) =  2*spade::consts::pi*delta;
    
    spade::coords::identity<real_t> coords;
    spade::grid::cartesian_grid_t grid(num_blocks, cells_in_block, exchange_cells, bounds, coords, group);
    
    prim_t fill1 = 0.0;
    spade::grid::grid_array prim(grid, fill1);

    // Eww
    std::vector<std::string> convert_files;
    bool begun = false;
    bool ended = false;
    for (const auto& s: control.get_raw_args())
    {
        if (s == "-files")
        {
            begun = true;
        }
        if ((s != "-files") && (s[0] == '-'))
        {
            ended = true;
        }
        if (begun && !ended && (s != "-files"))
        {
            convert_files.push_back(s);
        }
        
    }
    int idx = 0;
    
    spade::cli_args::shortname_args_t args(argc, argv);
    std::string outdir = "output";
    if (args.has_arg("-outdir"))
    {
        outdir = args["-outdir"];
    }
    for (const auto& fname : convert_files)
    {
        if (group.isroot()) print("Converting file", fname);
        spade::io::binary_read(fname, prim);
        std::string fformat = "out" + spade::utils::zfill(idx, 8);
        spade::io::output_vtk(outdir, fformat, prim);
        idx++;
    }
    return 0;
}
