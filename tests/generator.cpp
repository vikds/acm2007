#include <fstream>
#include <functional>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <queue>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

const int MAX_VALUE = std::numeric_limits<int>::max();

struct Options
{
    std::string output_file;
    size_t swamp_width;
    size_t points_number;
    bool print_help_only;

    Options(int argc, char** argv)
      : print_help_only(false)
    {
        namespace bpo = boost::program_options;
        bpo::options_description desc("Swamp problem. Options");

        desc.add_options()
            ("output_file,o", bpo::value<std::string>(&output_file)->default_value("input.txt"),
             "Path to output file. Output will be displayed on the screen if parameter is not specified.")
            ("swamp_width,s", bpo::value<size_t>(&swamp_width)->required(),
             "The width of a swamp. Integer value representing the Y coordinate. Opposite shore value coordinate is 0.")
            ("points_number,p", bpo::value<size_t>(&points_number)->required(),
             "The number of points that need to be place inside the swamp.")
            ("help,h", "shows this help");

        bpo::variables_map map;
        bpo::store(bpo::parse_command_line(argc, argv, desc), map);

        if (argc <= 1 || map.count("help"))
        {
            std::cout << desc << std::endl;
            print_help_only = true;
            return;
        }

        bpo::notify(map);
    }
};

int main(int argc, char* argv[])
{
    try
    {
        Options options(argc, argv);
        if (options.print_help_only)
        {
            return 0;
        }

        if (!options.output_file.empty() && boost::filesystem::exists(options.output_file))
        {
            std::cerr << "Ouput file already exists: " << options.output_file << std::endl;
            boost::filesystem::remove(options.output_file);
        }

        if (options.points_number == 0)
        {
            std::cerr << "Points number parameter must be specified: " << options.points_number << std::endl;
            return 1;
        }

        size_t width = options.swamp_width;
        size_t points = options.points_number;

        int plank_length = std::sqrt(static_cast<double>(4 * width * width) / points);

        // increase plank:
        plank_length *= 1.3;

        std::ofstream output_file(options.output_file);
        std::ostream& output = options.output_file.empty() ? std::cout : output_file;

        output << options.swamp_width << " ";
        output << options.points_number << " ";
        output << plank_length << std::endl;

        srand(time(nullptr));

        std::set<std::pair<size_t, size_t>> used;

        for (size_t point = 0; point < points; )
        {
            size_t x = ((static_cast<double>(rand()) / RAND_MAX) * width);
            size_t y = ((static_cast<double>(rand()) / RAND_MAX) * width);
            if (y == 0 || y == width)
            {
                continue;
            }

            if (used.find({x, y}) != used.end())
            {
                continue;
            }

            used.insert({x, y});
            output << x << " " << y << std::endl;
            point++;
        }

        return 0;
    }
    catch (const std::exception& ex)
    {
        std::cerr << "FATAL ERROR: " << ex.what() << std::endl;
        exit(1);
    }
}
