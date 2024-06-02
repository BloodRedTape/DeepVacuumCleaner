#pragma once

#include <SFML/System/Vector2.hpp>

constexpr float Eps = 0.001;

constexpr float PointRadius = 5.f;

constexpr float CleanerRadius = 60.f;
constexpr float CleanerRayLength = 40;
static sf::Vector2f CleanerSpeed(60, 10);

constexpr float MutationChance = 0.5;
constexpr float MutationRange = 100;
constexpr float GenofondMutationChance = 0.5;

constexpr size_t StandStillToDie = 30;
constexpr size_t IterationsPerGoal = 1000;

constexpr size_t ModelsToCrossover = 8;
constexpr size_t BestModelsToMutate = 5;
constexpr size_t BestModelsToMutationRange = 100;
constexpr size_t BestModelsToMutationChance = 0.5;
constexpr size_t BestModelsMutateTimes = 10;
constexpr size_t RandomCrossovers = 3;
constexpr size_t ModelsToSave = 30;
