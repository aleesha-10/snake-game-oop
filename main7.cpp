#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <iostream>
#include <vector>
#include <ctime> // For time handling
using namespace std;

#define SIZE 25
#define SCREEN_WIDTH SIZE * SIZE
#define SCREEN_HEIGHT SIZE * SIZE

#define MAX_OBSTACLES 10 // Maximum number of obstacles

enum SnakeDirection
{
    UP, DOWN, LEFT, RIGHT
};

SnakeDirection direction = SnakeDirection::RIGHT;

sf::SoundBuffer eatBuffer;
sf::Sound eatSound;
sf::Music backgroundMusic;

class Drawable
{
public:
    virtual void draw(sf::RenderWindow &window) const = 0; // Pure virtual function
};

class Border : public Drawable
{
private:
    sf::RectangleShape border;

public:
    Border(const sf::RenderWindow &window, float thickness, sf::Color borderColor)
    {
        border.setSize(sf::Vector2f(window.getSize().x - 2 * thickness, window.getSize().y - 2 * thickness));
        border.setPosition(thickness, thickness);
        border.setFillColor(sf::Color::Transparent);
        border.setOutlineThickness(thickness);
        border.setOutlineColor(borderColor);
    }

    void draw(sf::RenderWindow &window) const override
    {
        window.draw(border);
    }
};

class Apple : public Drawable
{
public:
    int x, y;

    Apple()
    {
        respawn();
    }

    void respawn()
    {
        x = rand() % SIZE;
        y = rand() % SIZE;
    }

    void draw(sf::RenderWindow &window) const override
    {
        sf::RectangleShape shape(sf::Vector2f(SIZE, SIZE));
        shape.setPosition(x * SIZE, y * SIZE);
        shape.setFillColor(sf::Color::Green); // Set apple color to green
        window.draw(shape);
    }
};

class Obstacle : public Drawable
{
private:
    vector<int> obstacleX;     // X-coordinates of obstacles
    vector<int> obstacleY;     // Y-coordinates of obstacles
    vector<sf::RectangleShape> obstacleShapes; // SFML rectangle shapes for obstacles
    vector<time_t> spawnTime;  // Time when each obstacle was created

public:
    Obstacle()
    {
        obstacleX.resize(MAX_OBSTACLES);
        obstacleY.resize(MAX_OBSTACLES);
        obstacleShapes.resize(MAX_OBSTACLES);
        spawnTime.resize(MAX_OBSTACLES);
    }

    void Initialize()
    {
        for (int i = 0; i < MAX_OBSTACLES; ++i)
        {
            spawnTime[i] = time(nullptr);
            obstacleX[i] = rand() % SIZE;
            obstacleY[i] = rand() % SIZE;

            obstacleShapes[i].setSize(sf::Vector2f(SIZE, SIZE));  // Set obstacle size
            obstacleShapes[i].setFillColor(sf::Color::Black);      // Set obstacle color to black
            obstacleShapes[i].setPosition(obstacleX[i] * SIZE, obstacleY[i] * SIZE); // Set position
        }
    }

    void Update()
    {
        for (int i = 0; i < MAX_OBSTACLES; ++i)
        {
            if (difftime(time(nullptr), spawnTime[i]) > 3)
            {
                if (rand() % 2 == 0)
                {
                    obstacleShapes[i].setFillColor(sf::Color::Red); // Show obstacle
                }
                else
                {
                    obstacleShapes[i].setFillColor(sf::Color::Red); // Hide obstacle
                }
                spawnTime[i] = time(nullptr);
            }
        }
    }

    void draw(sf::RenderWindow &window) const override
    {
        for (const auto &shape : obstacleShapes)
        {
            window.draw(shape);
        }
    }

    bool CheckCollision(int x, int y) const
    {
        for (int i = 0; i < MAX_OBSTACLES; ++i)
        {
            if (obstacleX[i] == x && obstacleY[i] == y)
            {
                return true;
            }
        }
        return false;
    }
};

class Snake : public Drawable
{
public:
    int size = 1;
    vector<sf::Vector2i> body;

    Snake()
    {
        body.push_back(sf::Vector2i(SIZE / 2, SIZE / 2)); // Initial position
    }

    void move()
    {
        for (int i = size - 1; i > 0; i--)
        {
            body[i] = body[i - 1];
        }

        switch (direction)
        {
        case UP:
            body[0].y -= 1;
            break;
        case DOWN:
            body[0].y += 1;
            break;
        case LEFT:
            body[0].x -= 1;
            break;
        case RIGHT:
            body[0].x += 1;
            break;
        }

        if (body[0].x >= SIZE)
            body[0].x = 0;
        else if (body[0].x < 0)
            body[0].x = SIZE - 1;
        else if (body[0].y >= SIZE)
            body[0].y = 0;
        else if (body[0].y < 0)
            body[0].y = SIZE - 1;
    }

    void draw(sf::RenderWindow &window) const override
    {
        sf::RectangleShape shape(sf::Vector2f(SIZE, SIZE));
        for (int i = 0; i < size; i++)
        {
            shape.setPosition(body[i].x * SIZE, body[i].y * SIZE);
            shape.setFillColor(sf::Color(200, 162, 200)); // Light purple
            window.draw(shape);
        }
    }

    Apple apple;
};

int main()
{
    sf::RenderWindow window(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), "SFML Snake");
    sf::Clock ticker;

    sf::Font font;
    if (!font.loadFromFile("arial.ttf"))
    {
        cout << "Error loading font.\n";
        return -1;
    }

    // Sound buffers
    sf::SoundBuffer eatBuffer;
    if (!eatBuffer.loadFromFile("eatSound.wav"))
    {
        cout << "Error loading eat sound.\n";
        return -1;
    }
    sf::Sound eatSound;
    eatSound.setBuffer(eatBuffer);

    sf::SoundBuffer collisionBuffer;
    if (!collisionBuffer.loadFromFile("collision_sound.wav"))
    {
        cout << "Error loading collision sound.\n";
        return -1;
    }
    sf::Sound collisionSound;
    collisionSound.setBuffer(collisionBuffer);

    sf::Music backgroundMusic;
    if (!backgroundMusic.openFromFile("background_music.wav"))
    {
        cout << "Error loading background music.\n";
        return -1;
    }
    backgroundMusic.setLoop(true); // Loop the background music
    backgroundMusic.setVolume(50); // Adjust volume (0 to 100)
    backgroundMusic.play();        // Start playing background music

    sf::Text pointsText;
    pointsText.setFont(font);
    pointsText.setString("Points: 0");
    pointsText.setCharacterSize(24);
    pointsText.setFillColor(sf::Color::White);
    pointsText.setPosition(10.f, 10.f);

    sf::Text livesText;
    livesText.setFont(font);
    livesText.setCharacterSize(24);
    livesText.setFillColor(sf::Color::White);
    livesText.setPosition(SCREEN_WIDTH - 150.f, 10.f);

    Snake snake;
    Obstacle obstacles;
    obstacles.Initialize();

    // Create a Border object
    Border border(window, 10, sf::Color::Yellow);

    // Lives variable
    int lives = 3;

    while (window.isOpen())
    {
        sf::Event e;
        while (window.pollEvent(e))
        {
            if (e.type == sf::Event::Closed)
                window.close();
            else if (e.type == sf::Event::KeyPressed)
            {
                switch (e.key.code)
                {
                case sf::Keyboard::Up:
                    if (direction != DOWN)
                        direction = UP;
                    break;
                case sf::Keyboard::Down:
                    if (direction != UP)
                        direction = DOWN;
                    break;
                case sf::Keyboard::Left:
                    if (direction != RIGHT)
                        direction = LEFT;
                    break;
                case sf::Keyboard::Right:
                    if (direction != LEFT)
                        direction = RIGHT;
                    break;
                }
            }
        }

        window.clear(sf::Color(211, 211, 211)); // Light gray background

        if (ticker.getElapsedTime().asMilliseconds() > 100)
        {
            snake.move();
            ticker.restart();
        }

        pointsText.setString("Points: " + std::to_string(snake.size - 1));

        // Border Collision Check (game over if snake goes out of bounds)
        if (snake.body[0].x < 0 || snake.body[0].x >= SIZE || snake.body[0].y < 0 || snake.body[0].y >= SIZE)
        {
            cout << "Game Over! Collision with border." << endl;
            lives--; // Decrease life by 1
            collisionSound.play(); // Play collision sound
            if (lives <= 0)
            {
                cout << "Game Over! No lives left." << endl;
                window.close();
            }
            snake.body.clear(); // Reset snake position
            snake.body.push_back(sf::Vector2i(SIZE / 2, SIZE / 2)); // Reset position to center
        }

        // Check collision with obstacles
        if (obstacles.CheckCollision(snake.body[0].x, snake.body[0].y))
        {
            cout << "Game Over! Collision with obstacle." << endl;
            lives--; // Decrease life by 1
            collisionSound.play(); // Play collision sound
            if (lives <= 0)
            {
                cout << "Game Over! No lives left." << endl;
                window.close();
            }
            snake.body.clear(); // Reset snake position
            snake.body.push_back(sf::Vector2i(SIZE / 2, SIZE / 2)); // Reset position to center
        }

        // Self-collision check
        for (int i = 1; i < snake.size; ++i)
        {
            if (snake.body[0] == snake.body[i])
            {
                cout << "Game Over! Self-collision." <<endl;
                window.close();
            }
        }

        // Check if the snake eats an apple
        if (snake.body[0].x == snake.apple.x && snake.body[0].y == snake.apple.y)
        {
            snake.apple.respawn(); // Respawn apple
            snake.size++;         // Increase snake size
            eatSound.play();      // Play eat sound
        }

        // Draw the game elements
        border.draw(window);
        snake.draw(window);
        snake.apple.draw(window);
        obstacles.draw(window);

        window.draw(pointsText);
        livesText.setString("Lives: " + std::to_string(lives));
        window.draw(livesText);

        window.display();
    }

    return 0;
}

