#include "stdafx.h"

#include <iostream>
#include <vector>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>

class Object
{
public:
    sf::IntRect hitbox;

    Object(sf::IntRect initialHitbox) : hitbox{ initialHitbox } {}
};

class Player
{
private:
public:
    const sf::Vector2f spawnPoint{ 100.0f, 950.0f };

    sf::FloatRect hitbox;
    sf::Vector2f velocity;

    bool pressingLeft{ false };
    bool pressingRight{ false };
    bool pressingJump{ false };
    bool isOnGround{ false };

    const int walkSpeed{ 100 };

    const float flightTime{ 0.75f };
    const float jumpHeight{ 80 };

    const float jumpVelocity;
    const float gravity;
    const float maximumDownwardVelocity;

    Player() :
        hitbox{ sf::FloatRect(spawnPoint.x, spawnPoint.y, 20, 28) },
        velocity{ sf::Vector2f(0.0f, 0.0f) },
        jumpVelocity{ 2 * jumpHeight * (2 / flightTime) },
        gravity{ (2 / flightTime) * jumpVelocity },
        maximumDownwardVelocity{ jumpVelocity }
    {
        std::cout << "gravity set to: " << gravity << std::endl;
        std::cout << "jumpVelocity set to: " << jumpVelocity << std::endl;
    }

    void inputs()
    {
        // dvorakians modify bindings here
        pressingLeft = sf::Keyboard::isKeyPressed(sf::Keyboard::A);
        pressingRight = sf::Keyboard::isKeyPressed(sf::Keyboard::D);
        pressingJump = sf::Keyboard::isKeyPressed(sf::Keyboard::Space);
    }

    void update(float timeStep, std::vector<Object>& objects)
    {
        {
            sf::FloatRect groundCheckingHitbox(hitbox);
            groundCheckingHitbox.top += 1;

            bool groundCheck{ false };

            for (Object object : objects)
            {
                sf::FloatRect objectHitbox(object.hitbox);
                if (groundCheckingHitbox.intersects(objectHitbox)) groundCheck = true;
            }

            isOnGround = groundCheck;
        }

        if (pressingLeft == pressingRight)
        {
            velocity.x = 0.0f;
        }
        else
        {
            if (pressingLeft) velocity.x = -walkSpeed;
            if (pressingRight) velocity.x = walkSpeed;
        }

        if (!isOnGround) velocity.y += gravity * timeStep;

        if (pressingJump && isOnGround)
        {
            velocity.y = -jumpVelocity;
            isOnGround = false;
        }

        if (velocity.y > maximumDownwardVelocity) velocity.y = maximumDownwardVelocity;

        sf::FloatRect movedHitbox{ hitbox };

        movedHitbox.left += velocity.x * timeStep;
        movedHitbox.top += velocity.y * timeStep;

        Object* yCollision{ nullptr };

        for (Object& object : objects)
        {
            sf::FloatRect objectHitbox(object.hitbox);
            if (movedHitbox.intersects(objectHitbox))
            {
                yCollision = &object;
            }
        }

        if (yCollision)
        {
            movedHitbox.top = yCollision->hitbox.top - hitbox.height;
            isOnGround = true;
            velocity.y = 0;
        }

        hitbox = movedHitbox;

        // for when you inevitably jump off the map
        if (hitbox.top > 1100 || hitbox.left + hitbox.width < -100 || hitbox.left > 600)
        {
            hitbox.left = spawnPoint.x;
            hitbox.top = spawnPoint.y;
            isOnGround = false;
        }
    }

    sf::Vector2f getCenter()
    {
        return sf::Vector2f(hitbox.left + hitbox.width / 2, hitbox.top + hitbox.height / 2);
    }
};

int main()
{
    Player player;

    std::vector<Object> objects;
    objects.push_back(Object(sf::IntRect(0, 1000, 500, 10)));
    objects.push_back(Object(sf::IntRect(200, 940, 300, 10)));
    objects.push_back(Object(sf::IntRect(300, 880, 200, 10)));
    objects.push_back(Object(sf::IntRect(400, 820, 100, 10)));
    objects.push_back(Object(sf::IntRect(100, 820, 100, 10)));

    const sf::Vector2f viewSize(512.0f, 384.0f);

    sf::RenderWindow window;
    window.create(sf::VideoMode(1024, 768), "physics", sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(60);
    window.setView(sf::View(player.getCenter(), viewSize));

    sf::Clock clock;

    while (true)
    {
        sf::Event polledEvent;
        while (window.pollEvent(polledEvent))
        {
            if (polledEvent.type == sf::Event::Closed) window.close();
        }

        float timeStep = clock.restart().asSeconds();

        player.inputs();
        player.update(timeStep, objects);

        window.clear(sf::Color::Black);

        {
            const float take{ 0.03f };
            const float keep{ 1.0f - take };

            sf::Vector2f previousCenter{ window.getView().getCenter() };
            sf::Vector2f playerCenter{ player.getCenter() };
            sf::Vector2f newCenter{ (previousCenter.x * keep) + (playerCenter.x * take),  (previousCenter.y * keep) + (playerCenter.y * take) };

            window.setView(sf::View(newCenter, viewSize));
        }

        for (Object object : objects)
        {
            sf::RectangleShape shape;
            shape.setSize(sf::Vector2f(object.hitbox.width, object.hitbox.height));
            shape.setPosition(sf::Vector2f(object.hitbox.left, object.hitbox.top));
            shape.setOutlineColor(sf::Color::White);
            shape.setFillColor(sf::Color::Transparent);
            shape.setOutlineThickness(1.0f);

            window.draw(shape);
        }

        {
            sf::RectangleShape shape;
            shape.setSize(sf::Vector2f(player.hitbox.width, player.hitbox.height));
            shape.setPosition(sf::Vector2f(player.hitbox.left, player.hitbox.top));
            shape.setOutlineColor(sf::Color::Red);
            shape.setFillColor(sf::Color::Transparent);
            shape.setOutlineThickness(1.0f);

            window.draw(shape);
        }

        window.display();

    }

    return 0;
}

