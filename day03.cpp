#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <set>
#include <memory>
#include <limits>
#include <unordered_map>
#include <unordered_set>
#include <boost/functional/hash.hpp>

#define UP      'U'
#define DOWN    'D'
#define LEFT    'L'
#define RIGHT   'R'

class Line 
{
public:
    Line(int x_1, int y_1, int x_2, int y_2) :
        _x_1 (x_1), _y_1 (y_1), _x_2 (x_2), _y_2 (y_2),
        _x_1_orig (x_1), _y_1_orig (y_1), _x_2_orig (x_2), _y_2_orig (y_2)
        
    {
        if (_x_1 > _x_2)
        {
            int tmp = _x_1;
            _x_1 = _x_2;
            _x_2 = tmp;
        }

        if (_y_1 > _y_2)
        {
            int tmp = _y_1;
            _y_1 = _y_2;
            _y_2 = tmp;
        }
    }
    int getDistance() { return _distance; }
    void setDistance(int distance) {_distance = distance;}
    virtual bool collide(const Line&) const = 0;
    virtual bool less_than(const Line&) const = 0;
    virtual bool isVertical() const = 0;
    virtual bool isHorizontal() const = 0;
    
    const int&  getX1() const { return _x_1; }
    const int&  getX2() const { return _x_2; }
    const int&  getY1() const { return _y_1; }
    const int&  getY2() const { return _y_2; }


    const int&  getX1O() const { return _x_1_orig; }
    const int&  getX2O() const { return _x_2_orig; }
    const int&  getY1O() const { return _y_1_orig; }
    const int&  getY2O() const { return _y_2_orig; }

    friend std::ostream& operator << (std::ostream& os, const Line& line)
    {
        os << "(" << line.getX1O() << ", " << line.getY1O() << ") -> (" 
            << line.getX2O() << ", " << line.getY2O() << ")";
    }

private:
    int _x_1, _y_1, _x_2, _y_2, _distance;
    int _x_1_orig, _y_1_orig, _x_2_orig, _y_2_orig;
};

class VerticalLine :
    public Line
{
public: 
    VerticalLine(int y_1, int y_2, int x) :
        Line(x, y_1, x, y_2)
    { }

    virtual bool collide(const Line& line) const override
    {   
        if (line.isHorizontal())
            return getY1() <= line.getY1() && line.getY1() <= getY2() &&
                line.getX1() <= getX1() && getX1() <= line.getX2();
        else return false;
    }

    virtual bool less_than(const Line& line) const override
    {
        if (line.isVertical())
            return getY1() <= line.getY1();
        else 
            return false;
    }

    virtual bool isVertical() const override { return true; }
    virtual bool isHorizontal() const override { return false; }

};

class HorizontalLine :
    public Line
{
public:
    HorizontalLine(int x_1, int x_2, int y) :
        Line(x_1, y, x_2, y)
    { }

    virtual bool collide(const Line& line) const override
    {
        if (line.isVertical())
            return getX1() <= line.getX1()  && line.getX1() <= getX2()  &&
                line.getY1() <= getY1() && getY1() <= line.getY2();
        else return false;
    }

    virtual bool less_than(const Line& line) const override
    {
        if (line.isHorizontal())
            return getX1() <= line.getX1();
        else return false;
    }

    virtual bool isVertical() const override { return false; }
    virtual bool isHorizontal() const override { return true; }
};

struct LineCompare
{
    bool operator() (const Line& lhs, const Line& rhs)
    {
        return lhs.less_than(rhs);
    }

    bool operator() (const std::unique_ptr<Line>& lhs, const std::unique_ptr<Line>& rhs)
    {
        return lhs->less_than(*rhs);
    }
};

void initializeFirstLine(
    std::string& line, 
    std::set<VerticalLine, LineCompare>& verticalLines, 
    std::set<HorizontalLine, LineCompare>& horizontalLines)
{
    std::stringstream ss(line);
    std::string token;
    int coords_x = 0, coords_y = 0;
    int distanceTotal = 0;
    while (getline(ss, token, ','))
    {
        int newCoords_x = coords_x;
        int newCoords_y = coords_y;
        
        int distance = stoi(token.substr(1, token.size() - 1));
        switch (token.at(0)) {
            case RIGHT:
                newCoords_x += distance;
                break;

            case LEFT:
                newCoords_x -= distance;
                break;

            case UP:
                newCoords_y += distance;
                break;

            case DOWN:
                newCoords_y -= distance;
                break; 
        }

        distanceTotal += distance;
        if (token.at(0) == UP || token.at(0) == DOWN)
        {
            VerticalLine l(coords_y, newCoords_y, newCoords_x);
            l.setDistance(distanceTotal);
            verticalLines.insert(l);
        }
        else
        {
            HorizontalLine l(coords_x, newCoords_x, newCoords_y);
            l.setDistance(distanceTotal);
            horizontalLines.insert(l);
        }
        coords_x = newCoords_x;
        coords_y = newCoords_y;
    }
}

struct pair_hashint
{
	template <class T1, class T2>
	std::size_t operator() (const std::pair<T1, T2> &pair) const
	{
		return std::hash<T1>()(pair.first) ^ std::hash<T2>()(pair.second);
	}
};

int getClosestCollision(
    std::string& line, 
    std::set<VerticalLine, LineCompare>& verticalLines, 
    std::set<HorizontalLine, LineCompare>& horizontalLines)
{
    std::stringstream ss(line);
    std::string token;
    int coords_x = 0, coords_y = 0, min_dist = std::numeric_limits<int>::max();
    // std::cout << min_dist << std::endl;
    int distanceTotal = 0;
    int maxSignal = 0;
    std::unordered_set< 
        std::pair<int, int>, 
        boost::hash< std::pair<int, int> > 
    > distSet;
    while (getline(ss, token, ','))
    {
        int newCoords_x = coords_x;
        int newCoords_y = coords_y;
        int distance = stoi(token.substr(1, token.size() - 1));
        switch (token.at(0)) {
            case RIGHT:
                newCoords_x += distance;
                break;

            case LEFT:
                newCoords_x -= distance;
                break;

            case UP:
                newCoords_y += distance;
                break;

            case DOWN:
                newCoords_y -= distance;
                break; 
        }
        distanceTotal += distance;
        
        if (token.at(0) == UP || token.at(0) == DOWN)
        {
            auto line = VerticalLine(coords_y, newCoords_y, newCoords_x);
            line.setDistance(distanceTotal);
            for (auto hLine : horizontalLines) {
                if (line.collide(hLine))
                {
                    int dist = abs(line.getX2()) + abs(hLine.getY2());
                    if (dist == 0)
                        continue;
                    
                    min_dist = dist < min_dist ? dist : min_dist;
                    std::cout << "Minimum distance is: " << min_dist << std::endl;
                    auto ptr = distSet.emplace(std::pair<int, int> (line.getX2(), line.getY2()));
                    //if (ptr.second)
                    //{
                        std::cout << line << ", " << hLine << std::endl;
                        std::cout << hLine.getDistance() << std::endl;
                        std::cout << hLine.getX2O() - line.getX2() << std::endl;
                        std::cout << line.getDistance() << std::endl;
                        std::cout << line.getY2O() - hLine.getY2() << std::endl;
                        
                        int signal = hLine.getDistance() - abs(hLine.getX2O() - line.getX2())
                            + line.getDistance() - abs(line.getY2O() - hLine.getY2()) ;
                        
                        if (maxSignal < signal)
                            maxSignal = signal;
                        std::cout << "Inserted! Max signal is: " << maxSignal << std::endl;;
                    //}
                }
            }
        }
        else 
        {
            auto line = HorizontalLine(coords_x, newCoords_x, newCoords_y);
            line.setDistance(distanceTotal);
            for (auto vLine : verticalLines) {
                if (line.collide(vLine))
                {
                    int dist = abs(line.getY2()) + abs(vLine.getX2());
                    if (dist == 0)
                        continue;
                    min_dist = dist < min_dist ? dist : min_dist;
                    std::cout << "Minimum distance is:"  << min_dist << std::endl;
                    auto ptr = distSet.emplace(std::pair<int, int> (line.getX2(), line.getY2()));
                    //if (ptr.second)
                    //{
                        std::cout << line << ", zasd" << vLine << std::endl;
                        std::cout << line.getDistance() << std::endl;
                        std::cout << line.getX2O() - vLine.getX2() << std::endl;
                        std::cout << vLine.getDistance() << std::endl;
                        std::cout << vLine.getY2O() - line.getY2() << std::endl;
                        int signal = line.getDistance() - abs(line.getX2O() - vLine.getX2())
                            + vLine.getDistance() - abs(vLine.getY2O() - line.getY2()) ;
                        
                        if (maxSignal < signal)
                            maxSignal = signal;
                        std::cout << "Inserted! Max signal is; " << maxSignal << std::endl;
                    //}
                }
            }
        }
        coords_x = newCoords_x;
        coords_y = newCoords_y;
    }
    return min_dist;
}

int main () 
{
    std::fstream inputFile("day03.txt");
    std::string lines;
    getline(inputFile, lines);

    std::set<VerticalLine, LineCompare> verticalLines;
    std::set<HorizontalLine, LineCompare> horizontalLines;
    initializeFirstLine(lines, verticalLines, horizontalLines);

    getline(inputFile, lines);
    std::cout << "Closest collision is: " 
        << getClosestCollision(lines, verticalLines, horizontalLines) << std::endl;

    return 0;
}