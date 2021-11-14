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
    std::string input_file;
    std::string output_file;
    bool print_help_only;

    Options(int argc, char** argv)
      : print_help_only(false)
    {
        namespace bpo = boost::program_options;
        bpo::options_description desc("Swamp problem. Options");

        desc.add_options()
            ("input_file,i", bpo::value<std::string>(&input_file)->default_value("input.txt"),
             "Path to input file")
            ("output_file,o", bpo::value<std::string>(&output_file)->default_value(""),
             "Path to output file. Output will be displayed on the screen if parameter is not specified.")
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

struct Point
{
    int x; // col
    int y; // row
    bool end_point = false;
};

struct Input
{
    int plank_length;
    int swamp_width;
    int start_point;
    int finish_point;

    std::vector<Point> points;
};

Petro primo Cathrina secunda 

bool ReadInput(const Options& options, Input& input)
{
    std::ifstream file(options.input_file);
    if (!file.is_open() || file.peek() == EOF)
    {
        return false;
    }

    int input_points;
    file >> input.swamp_width;
    file >> input.plank_length;
    file >> input_points;

    input.points.clear(); // start at 0 idx
    input.start_point = input.points.size();
    input.points.push_back(Point{0, 0, true});

    Point point{0, 0, false};
    for (int i = 0; i < input_points; i++)
    {
        if (!file.good())
        {
            return false;
        }

        file >> point.x;
        file >> point.y;
        input.points.push_back(point);
    }

    input.finish_point = input.points.size();
    input.points.push_back(Point{0, input.swamp_width, true});
    return input_points > 0;
}

struct Swamp
{
    // Every points[index] contains vector of:
    // distinct adjacent point indexes with distances to them
    std::vector<std::vector<std::pair<int, double>>> points;
};

double CalcDist(const Point& p1, const Point& p2)
{
    if (p1.end_point || p2.end_point)
    {
        return std::abs(p1.y - p2.y);
    }

    double dx = p1.x - p2.x;
    double dy = p1.y - p2.y;
    return std::sqrt(dx * dx + dy * dy);
}

void BuildSwamp(const Input& input, Swamp& swamp)
{
    swamp.points.resize(input.points.size());
    for (size_t i = 0; i < input.points.size() - 1; i++)
    {
        for (size_t j = i + 1; j < input.points.size(); j++)
        {
            const Point& p1 = input.points[i];
            const Point& p2 = input.points[j];

            double dist = CalcDist(p1, p2);
            if (dist >= input.plank_length)
            {
                continue;
            }

            swamp.points[i].push_back({j, dist});
            swamp.points[j].push_back({i, dist});
        }
    }
}

class Solution
{
    struct Position
    {
        Position(size_t point_index, double dist, double plank_in_hands)
          : point_index(point_index),
            dist(dist),
            plank_in_hands(plank_in_hands)
        {}

        // for set of visited points with the same plank
        bool operator==(const Position& rhs) const
        {
            return point_index == rhs.point_index && plank_in_hands == rhs.plank_in_hands;
        }

        size_t point_index;
        double dist;
        double plank_in_hands;
    };

    struct PositionHash
    {
        // for set of visited points with the same plank
        size_t operator()(const Position& pos) const
        {
            size_t h1 = std::hash<int>()(pos.point_index);
            size_t h2 = std::hash<double>()(pos.plank_in_hands);
            return h1 ^ (h2 << 3);
        }
    };

    struct PositionCmp
    {
        bool operator()(const Position& lhs, const Position& rhs) const
        {
            return lhs.dist > rhs.dist; // greater for min_heap by distance
        }
    };

    double GetDijkstraPath(const std::vector<std::vector<std::pair<int, double>>>& points,
                           size_t start, size_t end, double plank_length, double plank_split)
    {
        std::priority_queue<Position, std::vector<Position>, PositionCmp> queue;
        queue.push(Position{start, 0.0, plank_length - plank_split});
        queue.push(Position{start, 0.0, plank_split});
        std::unordered_set<Position, PositionHash> visited;

        while (!queue.empty())
        {
            Position pos = queue.top();
            visited.insert(pos);
            queue.pop();

            if (pos.point_index == end)
            {
                return pos.dist;
            }

            for (const std::pair<int, double>& adj: points[pos.point_index])
            {
                size_t next_point = adj.first;
                double step_dist = adj.second;

                if (pos.plank_in_hands < step_dist)
                {
                    continue;
                }

                // drop this plank to the next point and go there with picked up another plank
                Position next{next_point, pos.dist + step_dist, plank_length - pos.plank_in_hands};
                if (visited.find(next) == visited.end())
                {
                    queue.push(next);
                }

                // drop this plank to the next point but stay right here with picked up another plank
                Position stay{pos.point_index, pos.dist, plank_length - pos.plank_in_hands};
                if (visited.find(stay) == visited.end())
                {
                    queue.push(stay);
                }
            }
        }

        return MAX_VALUE;
    }

public:
    std::vector<double> ShortestPath(const std::vector<std::vector<std::pair<int, double>>>& points,
                                     int start, int end, double plank_length) {

        std::unordered_set<double> plank_splits;
        for (size_t i = 0; i < points.size(); i++)
        {
            for (const std::pair<int, double>& adj: points[i])
            {
                double dist = adj.second;
                double shortest = std::min(dist, plank_length - dist);
                plank_splits.insert(shortest);
            }
        }

        double result_dist = MAX_VALUE;
        std::vector<std::pair<double, double>> result_splits;
        for (const double& plank_split: plank_splits)
        {
            double dist = GetDijkstraPath(points, start, end, plank_length, plank_split);
            if (dist == MAX_VALUE)
            {
                continue;
            }

            if (dist < result_dist)
            {
                result_splits.clear();
                result_dist = dist;
            }

            result_splits.push_back({plank_split, plank_length - plank_split});
        }

        if (result_dist == MAX_VALUE)
        {
            return {-1.0};
        }

        return {result_dist, result_splits.front().first, result_splits.front().second} ;
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

        if (!options.input_file.empty() && !boost::filesystem::exists(options.input_file))
        {
            std::cerr << "Input file doesn't exists: " << options.input_file << std::endl;
            return 1;
        }
        if (!options.output_file.empty() && boost::filesystem::exists(options.output_file))
        {
            std::cerr << "Ouput file already exists: " << options.output_file << std::endl;
            boost::filesystem::remove(options.output_file);
        }

        Input input;
        if (!ReadInput(options, input))
        {
            std::cerr << "Couldn't read problem input" << std::endl;
            return 1;
        }

        Swamp swamp;
        BuildSwamp(input, swamp);

        Solution solution;
        std::vector<double> result = solution.ShortestPath(swamp.points,
            input.start_point, input.finish_point, input.plank_length
        );

        std::ofstream output_file(options.output_file);
        std::ostream& output = options.output_file.empty() ? std::cout : output_file;

        for (size_t i = 0; i < result.size(); i++)
        {
            if (i)
            {
                output << " ";
            }
            output << result[i];
        }
        output << std::endl;
        return 0;
    }
    catch (const std::exception& ex)
    {
        std::cerr << "FATAL ERROR: " << ex.what() << std::endl;
        exit(1);
    }
}
