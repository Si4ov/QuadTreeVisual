
#include <cmath>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <SFML/Graphics.hpp>

using namespace std;




// Used to hold details of a point
template <typename PointType, typename = enable_if_t<is_arithmetic<PointType>::value>>
struct Point {

	PointType x;
	PointType y;
	Point(PointType _x, PointType _y)
	{
		x = _x;
		y = _y;
	}
	Point()
	{
		x = 0;
		y = 0;
	}
};

// The objects that we want stored in the quadtree
template <typename NodeType>
struct Node {
	Point<NodeType> pos;
	NodeType data;
	Node(Point<NodeType> _pos, NodeType _data)
	{
		pos = _pos;
		data = _data;
	}
	Node() { data = 0; }
};

// The main quadtree class
template <typename NodeType>
class Quad {
	//  Boundary of this node
	Point<NodeType> botLeft;
	Point<NodeType> topRight;


	// quads of the tree
	unique_ptr<Quad<NodeType>> topLeftTree;
	unique_ptr<Quad<NodeType>> topRightTree;
	unique_ptr<Quad<NodeType>> botLeftTree;
	unique_ptr<Quad<NodeType>> botRightTree;


	//CHANGING DEPTH OF QUAD
	int maxDepth;

	vector<Node<NodeType>*> nodes;

	

	public:
	void insert(Node<NodeType>*);
	vector<Node<NodeType>*> search(Point<NodeType>);
	bool inBoundary(Point<NodeType>) const;

	mutable Quad<NodeType>* highlightedQuad = nullptr;


	void drawRecursive(sf::RenderWindow& window, sf::Font& font, sf::Vector2i mousePos, Quad<NodeType>*& highlightedQuad, int currentDepth) const {
		// Check if the quad is highlighted by mouse position and within the maxDepth
		if (inBoundary(Point<NodeType>(mousePos.x, mousePos.y)) && maxDepth >= 0 && currentDepth <= maxDepth) {
			// Draw the highlighted quad
			sf::RectangleShape shape(sf::Vector2f(topRight.x - botLeft.x, topRight.y - botLeft.y));
			shape.setPosition(botLeft.x, botLeft.y);
			shape.setFillColor(sf::Color::Transparent);
			shape.setOutlineColor(sf::Color::Green);
			shape.setOutlineThickness(2.f);
			window.draw(shape);

			// Store the highlighted quad
			highlightedQuad = const_cast<Quad<NodeType>*>(this);

			// Draw nodes within the highlighted quad
			for (const auto& node : nodes) {
				sf::CircleShape nodeShape(5.f);
				nodeShape.setPosition(node->pos.x - 5.f, node->pos.y - 5.f);
				nodeShape.setFillColor(sf::Color::Red);
				window.draw(nodeShape);

				// Display node name with font
				sf::Text text(std::to_string(node->data), font, 12);
				text.setPosition(node->pos.x + 10.f, node->pos.y - 10.f);
				window.draw(text);
			}
		}

		// Recursively draw the child quads
		if (topLeftTree != nullptr) topLeftTree->drawRecursive(window, font, mousePos, highlightedQuad, currentDepth );
		if (topRightTree != nullptr) topRightTree->drawRecursive(window, font, mousePos, highlightedQuad, currentDepth );
		if (botLeftTree != nullptr) botLeftTree->drawRecursive(window, font, mousePos, highlightedQuad, currentDepth );
		if (botRightTree != nullptr) botRightTree->drawRecursive(window, font, mousePos, highlightedQuad, currentDepth );
	}



public:

	void draw(sf::RenderWindow& window, sf::Font& font, sf::Vector2i mousePos, Quad<NodeType>*& highlightedQuad) const {
		drawRecursive(window, font, mousePos, highlightedQuad, 0);
	}

	Quad()
	{
		botLeft = Point<NodeType>(0, 0);
		topRight = Point<NodeType>(0, 0);
		topLeftTree = NULL;
		topRightTree = NULL;
		botLeftTree = NULL;
		botRightTree = NULL;
		highlightedQuad = NULL;
	}
	Quad(Point<NodeType> botL, Point<NodeType> topR, int maxDepthData)
	{
		maxDepth = maxDepthData;
		topLeftTree = NULL;
		topRightTree = NULL;
		botLeftTree = NULL;
		botRightTree = NULL;
		highlightedQuad = NULL;
		botLeft = botL;
		topRight = topR;
	}
	~Quad() = default;

};

// Check if current quadtree contains the point
template <typename NodeType>
bool Quad<NodeType>::inBoundary(Point<NodeType> p) const
{
	return (p.x >= botLeft.x && p.x <= topRight.x
		&& p.y >= botLeft.y && p.y <= topRight.y);
}

// Insert a node into the quadtree
template <typename NodeType>
void Quad<NodeType>::insert(Node<NodeType>* node)
{
	if (node == NULL)
		return;

	// Current quad cannot contain it
	if (!inBoundary(node->pos))
		return;

	for (Node<NodeType>* existingNode : nodes) {
		if (existingNode->pos.x == node->pos.x && existingNode->pos.y == node->pos.y) {
			// Node with the same coordinates found, store in vector
			nodes.push_back(node);
			return;
		}
	}
	if (maxDepth == 0) {
		nodes.push_back(node);
		return;
	}


	// We are at a quad of unit area
	// We cannot subdivide this quad further
	if (abs(botLeft.x - topRight.x) <= 1
		&& abs(botLeft.y - topRight.y) <= 1) {
		nodes.push_back(node);
		return;
	}


	if ((botLeft.x + topRight.x) / 2 >= node->pos.x) {
		// Indicates BotLeftTree
		if ((botLeft.y + topRight.y) / 2 >= node->pos.y) {
			if (botLeftTree == NULL)
				botLeftTree = make_unique<Quad>(
					Point<NodeType>(botLeft.x, botLeft.y),
					Point<NodeType>((botLeft.x + topRight.x) / 2, (botLeft.y + topRight.y) / 2),
					maxDepth - 1);
			botLeftTree->insert(node);
		}

		// Indicates TopLeftTree
		else {
			if (topLeftTree == NULL)
				topLeftTree = make_unique<Quad>(
					Point<NodeType>(botLeft.x, (botLeft.y + topRight.y) / 2),
					Point<NodeType>((botLeft.x + topRight.x) / 2, topRight.y),
					maxDepth - 1);
			topLeftTree->insert(node);
		}
	}
	else {
		// Indicates BotRightTree
		if ((botLeft.y + topRight.y) / 2 >= node->pos.y) {
			if (botRightTree == NULL)
				botRightTree = make_unique<Quad>(
					Point<NodeType>((botLeft.x + topRight.x) / 2, botLeft.y),
					Point<NodeType>(topRight.x, (botLeft.y + topRight.y) / 2),
					maxDepth - 1);
			botRightTree->insert(node);
		}

		// Indicates TopRightTree
		else {
			if (topRightTree == NULL)
				topRightTree = make_unique<Quad>(
					Point<NodeType>((botLeft.x + topRight.x) / 2, (botLeft.y + topRight.y) / 2),
					Point<NodeType>(topRight.x, topRight.y),
					maxDepth - 1);
			topRightTree->insert(node);
		}
	}
}

// Find a node in a quadtree
template <typename NodeType>
vector<Node<NodeType>*> Quad<NodeType>::search(Point<NodeType> p)
{
	// Current quad cannot contain it
	if (!inBoundary(p))
		return {};

	// We are at a quad of unit length
	// We cannot subdivide this quad further
	if (maxDepth == 0 || !nodes.empty())
		return nodes;

	if ((botLeft.x + topRight.x) / 2 >= p.x) {
		// Indicates topLeftTree
		if ((botLeft.y + topRight.y) / 2 >= p.y) {
			if (botLeftTree == NULL)
				return {};
			return botLeftTree->search(p);
		}

		// Indicates botLeftTree
		else {
			if (topLeftTree == NULL)
				return {};
			return topLeftTree->search(p);
		}
	}
	else {
		// Indicates topRightTree
		if ((botLeft.y + topRight.y) / 2 >= p.y) {
			if (botRightTree == NULL)
				return {};
			return botRightTree->search(p);
		}

		// Indicates botRightTree
		else {
			if (topRightTree == NULL)
				return {};
			return topRightTree->search(p);
		}
	}
};

template <typename NodeType>
void PrintFindedVector(vector<Node<NodeType>*> FindedData, int Num)
{
	cout << "Node " << Num << " data : ";
	for (int i = 0; i < FindedData.size(); i++)
	{
		cout << FindedData[i]->data << " ";
	}
	cout << "\n";

};

template <typename NodeType>
void MouseMethod(const Quad<NodeType>& center, sf::Vector2i mousePos)
{
	NodeType X = static_cast<NodeType>(mousePos.x);
	NodeType Y = static_cast<NodeType>(mousePos.y);
	if (center.highlightedQuad != nullptr) {
		vector<Node<NodeType>*> highlightedNodes = center.highlightedQuad->search({ X, Y });
		PrintFindedVector(highlightedNodes, 0);
	}
}

//CHANGE DATA TYPE 
using DataType = double;

const DataType WINDOW_WIDTH = 1000;
const DataType WINDOW_HEIGHT = 1000;



void GenerateData(Quad <DataType>& quad, int numNodes)
{
	srand(static_cast<unsigned int>(time(nullptr)));
	for (int i = 0; i < numNodes; ++i) {
		DataType x = rand() % 900 + 100;
		DataType y = rand() % 900 + 100;
		DataType value = i + 100;

		Node<DataType>* newNode = new Node<DataType>(Point<DataType>(x, y), value);

		quad.insert(newNode);
	}
}

int main()
{
	sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Quadtree Visualization");
	window.setFramerateLimit(60);

	Quad<DataType> center(Point<DataType>(0.0, 0.0), Point<DataType>(WINDOW_WIDTH, WINDOW_HEIGHT), 5);

	GenerateData(center, 20);

	// Load a font
	sf::Font font;
	if (!font.loadFromFile("C:/Windows/Fonts/arial.ttf")) {
		cerr << "Failed to load font\n";
		return EXIT_FAILURE;
	}

	while (window.isOpen()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed) {
				window.close();
			}
		}

		// Get mouse position
		sf::Vector2i mousePos = sf::Mouse::getPosition(window);

		//Clear of quads
		//window.clear();

		// Highlight the quad based on mouse position
		center.highlightedQuad = nullptr; 
		center.draw(window, font, mousePos, center.highlightedQuad);

		// If a quad is highlighted, display its data
		MouseMethod<DataType>(center, mousePos);

		window.display();
	}

	return 0;
}

