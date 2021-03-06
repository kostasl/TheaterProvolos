

/*******************************************************************
** This code is part of Breakout.
**
** Breakout is free software: you can redistribute it and/or modify
** it under the terms of the CC BY 4.0 license as published by
** Creative Commons, either version 4 of the License, or (at your
** option) any later version.
******************************************************************/
#include "game.h"
#include "resource_manager.h"
#include "particle_generator.h"
#include "sprite_renderer.h"
#include "ball_object.h"


SpriteRenderer       *Renderer;
ParticleGenerator    *Particles;
BallObject           *Ball;

// Initial velocity of the Ball
const glm::vec2 INITIAL_BALL_VELOCITY(100.0f, -350.0f);
// Radius of the ball object
const GLfloat BALL_RADIUS = 12.5f;



Game::Game(GLuint width, GLuint height)
    : State(GAME_ACTIVE), Keys(), Width(width), Height(height)
{

}


void Game::Init()
{

    // Load shaders
       ResourceManager::LoadShader("/home/kostasl/workspace/TheaterProvolos/theaterprovolos/res/sprite.vs", "/home/kostasl/workspace/TheaterProvolos/theaterprovolos/res/sprite.frag", nullptr, "sprite");
       // Configure shaders
       glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(this->Width), static_cast<GLfloat>(this->Height), 0.0f, -1.0f, 1.0f);
       ResourceManager::GetShader("sprite").Use().SetInteger("image", 0);
       ResourceManager::GetShader("sprite").SetMatrix4("projection", projection);
       // Load textures
       ResourceManager::LoadTexture("/home/kostasl/workspace/TheaterProvolos/theaterprovolos/res/textures/awesomeface.png", GL_TRUE, "face");

       // Set render-specific controls
       Shader shsprite = ResourceManager::GetShader("sprite");

       Renderer = new SpriteRenderer(shsprite);


       ResourceManager::LoadShader("/home/kostasl/workspace/TheaterProvolos/theaterprovolos/res/particle.vs", "/home/kostasl/workspace/TheaterProvolos/theaterprovolos/res/particle.frag", nullptr,"particle");
       ResourceManager::LoadTexture("/home/kostasl/workspace/TheaterProvolos/theaterprovolos/res/textures/particle.png", GL_TRUE, "particle");
       Shader shparticle = ResourceManager::GetShader("particle");

/// PARTICLES //


//    // Set render-specific controls
//    // Load textures
    GLuint nr_particles = 500;
    std::vector<Particle> particles;

    for (GLuint i = 0; i < nr_particles; ++i)
        particles.push_back(Particle());


    Texture2D txpart = ResourceManager::GetTexture("particle");
    Particles = new ParticleGenerator(
       shparticle,
           txpart,
           500
       );


//    /// BALL //
    Texture2D txface = ResourceManager::GetTexture("face");
    glm::vec2 ballPos = glm::vec2(50.0f,50.0f) + glm::vec2(50 / 2 - BALL_RADIUS, -BALL_RADIUS * 2);
        Ball = new BallObject(ballPos, BALL_RADIUS, INITIAL_BALL_VELOCITY,
            txface);


}

void Game::Update(GLfloat dt)
{

int Radius = 10;
    // Update particles
    Particles->Update(dt, *Ball, 2, glm::vec2(Radius / 2));
       Particles->Update(dt, *Ball, 2, glm::vec2(Ball->Radius / 2));
}


void Game::ProcessInput(GLfloat dt)
{

}

void Game::Render()
{


    Texture2D txface = ResourceManager::GetTexture("face");

     Renderer->DrawSprite(txface,
             glm::vec2(50, 50), glm::vec2(200, 200), 45.0f, glm::vec3(0.0f, 0.0f, 1.0f));

     Texture2D txpart = ResourceManager::GetTexture("particle");

     Renderer->DrawSprite(txpart,
            glm::vec2(80, 100), glm::vec2(120, 120), 5.0f, glm::vec3(1.0f, 0.0f, 0.0f));

    //    if (this->State == GAME_ACTIVE)      {

           // Draw player
            //Player->Draw(*Renderer);

//           // Draw particles
           Particles->Draw();
//           // Draw ball
          // Ball->Draw(*Renderer);
//       }



}



Game::~Game()
{

}
