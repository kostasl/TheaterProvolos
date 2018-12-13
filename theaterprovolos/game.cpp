

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

Game::Game(GLuint width, GLuint height)
    : State(GAME_ACTIVE), Keys(), Width(width), Height(height)
{

}

Game::~Game()
{

}

SpriteRenderer  *Renderer;
ParticleGenerator   *Particles;

void Game::Init()
{
    ResourceManager::LoadShader("shaders/sprite.vs", "shaders/sprite.frag", nullptr, "sprite");
    // Configure shaders
    glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(this->Width),
    static_cast<GLfloat>(this->Height), 0.0f, -1.0f, 1.0f);
    ResourceManager::GetShader("sprite").Use().SetInteger("image", 0);
    ResourceManager::GetShader("sprite").SetMatrix4("projection", projection);
    // Set render-specific controls
    Shader shsprite = ResourceManager::GetShader("sprite");
    Renderer = new SpriteRenderer(shsprite);
    // Load textures
    ResourceManager::LoadTexture("textures/awesomeface.png", GL_TRUE, "face");


    ResourceManager::LoadShader("res/particle.vs", "res/particle.frag", nullptr,"particle");

    ResourceManager::LoadTexture("res/textures/particle.png", GL_TRUE, "particle");
    GLuint nr_particles = 500;
    std::vector<Particle> particles;

  //  for (GLuint i = 0; i < nr_particles; ++i)
   //     particles.push_back(Particle());

    Particles = new ParticleGenerator(
           ResourceManager::GetShader("particle"),
           ResourceManager::GetTexture("particle"),
           500
       );

}

void Game::Update(GLfloat dt)
{

int Radius = 10;
    // Update particles
  //  Particles->Update(dt, *Ball, 2, glm::vec2(Radius / 2));
}


void Game::ProcessInput(GLfloat dt)
{

}

void Game::Render()
{
     Texture2D txface = ResourceManager::GetTexture("face");

    Renderer->DrawSprite(txface,
           glm::vec2(200, 200), glm::vec2(300, 400), 45.0f, glm::vec3(0.0f, 1.0f, 0.0f));

    if (this->State == GAME_ACTIVE)
       {

           // Draw player
           //Player->Draw(*Renderer);
           // Draw particles
           Particles->Draw();
           // Draw ball
           //Ball->Draw(*Renderer);
       }

}

