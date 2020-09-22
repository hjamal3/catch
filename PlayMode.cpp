#include "PlayMode.hpp"

#include "LitColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <random>

// for string formatting
#include <iomanip>
#include <sstream>

GLuint hexapod_meshes_for_lit_color_texture_program = 0;
Load< MeshBuffer > hexapod_meshes(LoadTagDefault, []() -> MeshBuffer const* {
	MeshBuffer const* ret = new MeshBuffer(data_path("arena.pnct"));
	hexapod_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
	});

Load< Scene > hexapod_scene(LoadTagDefault, []() -> Scene const* {
	return new Scene(data_path("arena.scene"), [&](Scene& scene, Scene::Transform* transform, std::string const& mesh_name) {
		Mesh const& mesh = hexapod_meshes->lookup(mesh_name);

		scene.drawables.emplace_back(transform);
		Scene::Drawable& drawable = scene.drawables.back();

		drawable.pipeline = lit_color_texture_program_pipeline;

		drawable.pipeline.vao = hexapod_meshes_for_lit_color_texture_program;
		drawable.pipeline.type = mesh.type;
		drawable.pipeline.start = mesh.start;
		drawable.pipeline.count = mesh.count;

		});
	});

PlayMode::PlayMode() : scene(*hexapod_scene) {
	//get pointers to leg for convenience:
	for (auto& transform : scene.transforms) {
/*		if (transform.name == "Hip.FL") hip = &transform;
		else if (transform.name == "UpperLeg.FL") upper_leg = &transform;
		else if (transform.name == "LowerLeg.FL") lower_leg = &transform;*/		
		if (transform.name == "Base") {
			base = &transform;
		} else if (transform.name == "Cone") {
			 thruster = &transform;
		} else if (transform.name == "Cart") {
			cart = &transform;
		} else if (transform.name == "Thruster") {
			thruster = &transform;
		}
		else if (transform.name == "Ball") {
			ball = &transform;
		}		
		else if (transform.name == "FixedBall") {
			fixed_ball = &transform;
		}
	}
	//if (hip == nullptr) throw std::runtime_error("Hip not found.");
	//if (upper_leg == nullptr) throw std::runtime_error("Upper leg not found.");
	//if (lower_leg == nullptr) throw std::runtime_error("Lower leg not found.");
	//hip_base_rotation = hip->rotation;
	//upper_leg_base_rotation = upper_leg->rotation;
	//lower_leg_base_rotation = lower_leg->rotation;	
	if (base == nullptr) throw std::runtime_error("base not found.");
	if (cart == nullptr) throw std::runtime_error("cart not found.");
	if (thruster == nullptr) throw std::runtime_error("thruster not found.");
	if (ball == nullptr) throw std::runtime_error("ball not found.");
	if (fixed_ball == nullptr) throw std::runtime_error("fixed ball not found.");

	base_rotation = base->rotation;
	thruster_rotation = thruster->rotation;

	//get pointer to camera for convenience:
	if (scene.cameras.size() != 1) throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
	camera = &scene.cameras.front();
}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const& evt, glm::uvec2 const& window_size) {
	// keyboard, based on https://www.libsdl.org/release/SDL-1.2.15/docs/html/guideinputkeyboard.html
	// velocity stuff based on ChiliTomatoNoodle C++ tutorials: https://www.youtube.com/watch?v=kOsnq5JJvaw&list=PLqCJpWy5Fohfil0gvjzgdV4h29R9kDKtZ
	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_ESCAPE) {
			SDL_SetRelativeMouseMode(SDL_FALSE);
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_a) {
			cart_velocity.x = -1;
			left.downs += 1;
			left.pressed = true;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_d) {
			cart_velocity.x = 1;
			right.downs += 1;
			right.pressed = true;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_w) {
			cart_velocity.y = 1;
			up.downs += 1;
			up.pressed = true;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_s) {
			cart_velocity.y = -1;
			down.downs += 1;
			down.pressed = true;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_SPACE)
		{
			if (!ball_drop)
			{
				glm::mat4x3 ball_to_world = fixed_ball->make_local_to_world();
				glm::mat3 rotm(ball_to_world[0], ball_to_world[1], ball_to_world[2]);
				ball_velocity = rotm * ball_shot_velocity;
				ball_velocity = glm::normalize(ball_velocity);
				ball_velocity *= ball_speed;
				ball->position = ball_to_world[3];
				ball_drop = true;
			}
			if (game_over)
			{
				ball->position.z = -2;
				game_over = false;
				ball_drop = false;

				// reset if the game ended
				if (points == 5)
				{
					points = 0;
					total_time = 0.0f;
				}
			}
		}
	}
	else if (evt.type == SDL_KEYUP) {
		if (evt.key.keysym.sym == SDLK_a) {
			if (cart_velocity.x < 0)
			{
				cart_velocity.x = 0;
			}
			left.pressed = false;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_d) {
			if (cart_velocity.x > 0)
			{
				cart_velocity.x = 0;
			}
			right.pressed = false;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_w) {
			if (cart_velocity.y > 0)
			{
				cart_velocity.y = 0;
			}
			up.pressed = false;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_s) {
			if (cart_velocity.y < 0)
			{
				cart_velocity.y = 0;
			}
			down.pressed = false;
			return true;
		}
	}
	else if (evt.type == SDL_MOUSEBUTTONDOWN) {
		if (SDL_GetRelativeMouseMode() == SDL_FALSE) {
			SDL_SetRelativeMouseMode(SDL_TRUE);
			return true;
		}
	}
	else if (evt.type == SDL_MOUSEMOTION) {
		if (SDL_GetRelativeMouseMode() == SDL_TRUE) {
			glm::vec2 motion = glm::vec2(
				evt.motion.xrel / float(window_size.y),
				-evt.motion.yrel / float(window_size.y)
			);
			camera->transform->rotation = glm::normalize(
				camera->transform->rotation
				* glm::angleAxis(-motion.x * camera->fovy, glm::vec3(0.0f, 1.0f, 0.0f))
				* glm::angleAxis(motion.y * camera->fovy, glm::vec3(1.0f, 0.0f, 0.0f))
			);
			return true;
		}
	}

	return false;
}

void PlayMode::update(float elapsed) {

	//slowly rotates through [0,1):
	wobble += elapsed / 10.0f;
	wobble -= std::floor(wobble);
	//glm::radians(3 * std::sin(wobble * 2.0f * float(M_PI))),

	base->rotation = base_rotation * glm::angleAxis(
		glm::radians(10 * std::sin(wobble * 2.0f * float(M_PI))),
	glm::vec3(0.0f, 0.0f, 1.0f)
	);	
	thruster->rotation = thruster_rotation * glm::angleAxis(
		glm::radians(20 * std::sin(wobble * 2.0f * float(M_PI))),
	glm::vec3(0.0f, 1.0f, 0.0f)
	);

	if (!game_over)
	{
		// move cart first
		//make it so that moving diagonally doesn't go faster:
		if (cart_velocity != glm::vec2(0.0f))
		{
			cart_velocity = glm::normalize(cart_velocity) * cart_speed;
		}
		cart->position.x += elapsed * cart_velocity.x;
		cart->position.y += elapsed * cart_velocity.y;

		//cart->position.x += move.x;
		//cart->position.y += move.y;

		// stop cart from going out of bounds
		if (cart->position.x - 0.5f < -arena_radius)
		{
			cart->position.x = -arena_radius + 0.5f;
		}
		else if (cart->position.x + 0.5f > arena_radius)
		{
			cart->position.x = arena_radius - 0.5f;
		}
		if (cart->position.y - 0.5f < -arena_radius)
		{
			cart->position.y = -arena_radius + 0.5f;
		}
		else if (cart->position.y + 0.5f > arena_radius)
		{
			cart->position.y = arena_radius - 0.5f;
		}

		if (ball_drop)
		{
			glm::vec3 pos = ball->position;
			////std::cout << pos.x << " " << pos.y << " " << pos.z << std::endl;
			ball->position += ball_velocity * elapsed;
			ball_velocity += ball_accel * elapsed;

			if (pos.z < ball_radius + cart_height)
			{
				if (pos.x > cart->position.x - cart_radius && pos.x < cart->position.x + cart_radius &&
					pos.y < cart->position.y + cart_radius && pos.y > cart->position.y - cart_radius)
				{
					game_over = true;
					points += 1;
					won = true;

				} else if (pos.z < ball_radius)
				{
					game_over = true;
					won = false;
				}
			}
		}

		// update total elapsed time
		total_time += elapsed;
	}

}

void PlayMode::draw(glm::uvec2 const& drawable_size) {
	//update camera aspect ratio for drawable:
	camera->aspect = float(drawable_size.x) / float(drawable_size.y);

	//set up light type and position for lit_color_texture_program:
	// TODO: consider using the Light(s) in the scene to do this
	glUseProgram(lit_color_texture_program->program);
	glUniform1i(lit_color_texture_program->LIGHT_TYPE_int, 1);
	GL_ERRORS();
	glUniform3fv(lit_color_texture_program->LIGHT_DIRECTION_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f, -1.0f)));
	GL_ERRORS();
	glUniform3fv(lit_color_texture_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 0.95f)));
	GL_ERRORS();
	glUseProgram(0);

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClearDepth(1.0f); //1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); //this is the default depth comparison function, but FYI you can change it.

	scene.draw(*camera);

	{ //use DrawLines to overlay some text:
		glDisable(GL_DEPTH_TEST);
		float aspect = float(drawable_size.x) / float(drawable_size.y);
		DrawLines lines(glm::mat4(
			1.0f / aspect, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		));

		constexpr float H = 0.09f;
		std::string draw_str;

		if (!game_over)
		{
			std::string points_str = std::to_string(points);
			draw_str = "WASD moves cart. Mouse motion rotates camera; escape ungrabs mouse. Points: " + points_str;
		}
		else if (won)
		{
			if (points == 5)
			{
				// convert time to nice printable string
				// https://stackoverflow.com/questions/29200635/convert-float-to-string-with-precision-number-of-decimal-digits-specified
				std::stringstream stream;
				stream << std::fixed << std::setprecision(2) << total_time;
				std::string time_str = stream.str();
				draw_str = "GAME OVER! You got 5 points in " + time_str + " seconds. Space to reset.";
			}
			else
			{
				std::string points_str = std::to_string(points);
				draw_str = "Gotttteeeeeem. Space to reset. Points: " + points_str;
			}
		}
		else
		{
			std::string points_str = std::to_string(points);
			draw_str = "Missed it. Space to reset. Points: " + points_str;

		}
		lines.draw_text(draw_str,
			glm::vec3(-aspect + 0.1f * H, -1.0 + 0.1f * H, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0x00, 0x00, 0x00, 0x00));
		float ofs = 2.0f / drawable_size.y;
		lines.draw_text(draw_str,
			glm::vec3(-aspect + 0.1f * H + ofs, -1.0 + +0.1f * H + ofs, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0xff, 0xff, 0xff, 0x00));

	}
}
