#include "Mode.hpp"

#include "Scene.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>

struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//----- game state -----

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up;

	//local copy of the game scene (so code can change it during gameplay):
	Scene scene;

	//hexapod leg to wobble:
	//Scene::Transform *hip = nullptr;
	//Scene::Transform *upper_leg = nullptr;
	//Scene::Transform *lower_leg = nullptr;
	//glm::quat hip_base_rotation;
	//glm::quat upper_leg_base_rotation;
	//glm::quat lower_leg_base_rotation;
	float wobble = 0.0f;

	Scene::Transform* base = nullptr;
	Scene::Transform* thruster = nullptr;
	Scene::Transform* fixed_ball = nullptr; // fixed inside the thruster
	Scene::Transform* ball = nullptr; 
	Scene::Transform* bullet = nullptr;
	Scene::Transform* cart = nullptr;
	glm::quat base_rotation;
	glm::quat thruster_rotation;

	const int arena_radius = 20;
	bool ball_drop = false;
	const glm::vec3 ball_shot_velocity = { -1, 0, 0 };
	const int ball_speed = 10;
	glm::vec3 ball_velocity;
	const glm::vec3 ball_accel = { 0,0,-9.8 };
	const float ball_radius = 0.5;

	glm::vec2 cart_velocity = glm::vec2(0.0f, 0.0f);
	const float cart_speed = 10.0f;
	const float cart_radius = 0.5f; // dimensions
	const float cart_height = 0.25f;

	// game state
	int points = 0;
	bool won = false;
	bool game_over = false;
	float total_time = 0.0f;

	//camera:
	Scene::Camera *camera = nullptr;

};
