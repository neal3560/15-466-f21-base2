#include "PlayMode.hpp"

#include "LitColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <random>

GLuint hexapod_meshes_for_lit_color_texture_program = 0;
Load< MeshBuffer > hexapod_meshes(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer const *ret = new MeshBuffer(data_path("hexapod.pnct"));
	hexapod_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

Load< Scene > hexapod_scene(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("hexapod.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
		Mesh const &mesh = hexapod_meshes->lookup(mesh_name);

		scene.drawables.emplace_back(transform);
		Scene::Drawable &drawable = scene.drawables.back();

		drawable.pipeline = lit_color_texture_program_pipeline;

		drawable.pipeline.vao = hexapod_meshes_for_lit_color_texture_program;
		drawable.pipeline.type = mesh.type;
		drawable.pipeline.start = mesh.start;
		drawable.pipeline.count = mesh.count;

	});
});

//pool loading functions
GLuint pool_meshes_for_lit_color_texture_program = 0;
Load< MeshBuffer > pool_meshes(LoadTagDefault, []() -> MeshBuffer const* {
	MeshBuffer const* ret = new MeshBuffer(data_path("pool.pnct"));
	pool_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

Load< Scene > pool_scene(LoadTagDefault, []() -> Scene const* {
	return new Scene(data_path("pool.scene"), [&](Scene& scene, Scene::Transform* transform, std::string const& mesh_name) {
		Mesh const& mesh = pool_meshes->lookup(mesh_name);

		scene.drawables.emplace_back(transform);
		Scene::Drawable& drawable = scene.drawables.back();

		drawable.pipeline = lit_color_texture_program_pipeline;

		drawable.pipeline.vao = pool_meshes_for_lit_color_texture_program;
		drawable.pipeline.type = mesh.type;
		drawable.pipeline.start = mesh.start;
		drawable.pipeline.count = mesh.count;

	});
});


PlayMode::PlayMode() : scene(*pool_scene) {
	//create and initialize all the balls
	for (auto& transform : scene.transforms) {
		if (transform.name == "Ball.1") balls[0] = new Ball(&transform);
		else if (transform.name == "Ball.2") balls[1] = new Ball(&transform);
		else if (transform.name == "Ball.3") balls[2] = new Ball(&transform);
		else if (transform.name == "Ball.4") balls[3] = new Ball(&transform);
		else if (transform.name == "Ball.5") balls[4] = new Ball(&transform);
		else if (transform.name == "Ball.6") balls[5] = new Ball(&transform);
		else if (transform.name == "Ball.7") balls[6] = new Ball(&transform);
		else if (transform.name == "Ball.8") balls[7] = new Ball(&transform);
		else if (transform.name == "Ball.9") balls[8] = new Ball(&transform);
		else if (transform.name == "Ball.10") balls[9] = new Ball(&transform);
		else if (transform.name == "Ball.11") balls[10] = new Ball(&transform);
		else if (transform.name == "Ball.12") balls[11] = new Ball(&transform);
		else if (transform.name == "Ball.13") balls[12] = new Ball(&transform);
		else if (transform.name == "Ball.14") balls[13] = new Ball(&transform);
		else if (transform.name == "Ball.15") balls[14] = new Ball(&transform);
	}

	//initialize ball counts
	num_balls = 15;

	//get pointer to camera for convenience:
	if (scene.cameras.size() != 1) throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
	camera = &scene.cameras.front();
}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_ESCAPE) {
			SDL_SetRelativeMouseMode(SDL_FALSE);
			return true;
		} else if (evt.key.keysym.sym == SDLK_a) {
			left.downs += 1;
			left.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			right.downs += 1;
			right.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			up.downs += 1;
			up.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			down.downs += 1;
			down.pressed = true;
			return true;
		}
	} else if (evt.type == SDL_KEYUP) {
		if (evt.key.keysym.sym == SDLK_a) {
			left.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			right.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			up.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			down.pressed = false;
			return true;
		}
	} 

	return false;
}

void PlayMode::update(float elapsed) {

	//ball size
	float ball_radius = 0.15f;

	//---------------- game status update --------------------

	//timer
	current_time += elapsed;

	//text
	std::string best_time_text;
	if (has_best) {
		best_time_text = std::to_string(best_time);
	}
	else {
		best_time_text = "No Record Exist";
	}

	Recordtext = "Best Record: " + best_time_text + "s  Current Time: " + std::to_string(current_time) + "s";

	//Game Over logic
	if (isGameOver) {
		for (size_t index = 0; index < 15; ++index) {
			balls[index]->reset();
		}
		isGameOver = false;
		num_balls = 15;
		current_time = 0.0f;
	}

	//------------ apply force on black ball --------------

	//combine inputs into a move:
	constexpr float PlayerForce = 2.0f;
	glm::vec3 force = glm::vec3(0.0f);
	if (left.pressed && !right.pressed) force.x = -1.0f;
	if (!left.pressed && right.pressed) force.x = 1.0f;
	if (down.pressed && !up.pressed) force.y = -1.0f;
	if (!down.pressed && up.pressed) force.y = 1.0f;

	//normalize force 
	if (force != glm::vec3(0.0f)) force = glm::normalize(force) * PlayerForce;

	//apply force
	balls[7]->velocity += force * elapsed;

	//------------------ target collision -------------------
	float target_radius = 0.4f;
	glm::vec3 targets[6] = {
		glm::vec3(-3.0f, -2.0f, 0.0f),
		glm::vec3(0.0f, -2.0f, 0.0f),
		glm::vec3(3.0f, -2.0f, 0.0f),
		glm::vec3(-3.0f, 2.0f, 0.0f),
		glm::vec3(0.0f, 2.0f, 0.0f),
		glm::vec3(3.0f, 2.0f, 0.0f)
	};
	
	for (size_t index = 0; index < 15; ++index) {

		//skip falling ball
		if (balls[index]->isOff) continue;

		for (size_t i = 0; i < 6; i++) {

			if (glm::length(balls[index]->transform->position - targets[i]) <= target_radius) {

				//set off flag to let the ball to ignore all collision
				balls[index]->isOff = true;

				//hide the falling ball
				balls[index]->transform->position.z -= 1.0f;

				//mother ball
				if (index == 7) {
					isGameOver = true;
					break;
				}

				//decrease number of balls
				num_balls--;
				if (num_balls == 1) {
					isGameOver = true;
					has_best = true;
					best_time = std::min(current_time, best_time);
					break;
				}

				//skip other target test if falling
				break;
			}
		}
	}

	//------------------ wall collision ---------------------

	//table size
	glm::vec2 table_radius = glm::vec2(3.0f, 2.0f);

	//table collision loss
	float table_ball_collision = 0.8f;

	for (size_t index = 0; index < 15; ++index) {

		//skip falling ball
		if (balls[index]->isOff) continue;

		//current ball position
		glm::vec3 cur_pos = balls[index]->transform->position;

		//left wall
		if (cur_pos.x - ball_radius < -table_radius.x) {
			balls[index]->transform->position.x = -table_radius.x + ball_radius;
			balls[index]->velocity.x = -table_ball_collision * balls[index]->velocity.x;
		} 

		//right wall
		if (cur_pos.x + ball_radius > table_radius.x) {
			balls[index]->transform->position.x = table_radius.x - ball_radius;
			balls[index]->velocity.x = -table_ball_collision * balls[index]->velocity.x;
		}

		//down wall
		if (cur_pos.y - ball_radius < -table_radius.y) {
			balls[index]->transform->position.y = -table_radius.y + ball_radius;
			balls[index]->velocity.y = -table_ball_collision * balls[index]->velocity.y;
		}

		//up wall
		if (cur_pos.y + ball_radius > table_radius.y) {
			balls[index]->transform->position.y = table_radius.y - ball_radius;
			balls[index]->velocity.y = -table_ball_collision * balls[index]->velocity.y;
		}
	}

	//---------------- balls' collision ---------------------

	//ball-ball collision loss
	float ball_ball_collision = 0.9f;

	//solve quadratic equation
	auto solve_quadratic_equation = [&](float a, float b, float c) {
		float discriminant = b * b - 4 * a * c;
		float t1 = (-b + std::sqrt(discriminant)) / (2 * a);
		float t2 = (-b - std::sqrt(discriminant)) / (2 * a);
		return std::max(t1, t2);
	};

	for (size_t i = 0; i < 15; ++i) {

		//skip falling ball
		if (balls[i]->isOff) continue;

		//position of the first ball
		glm::vec3 posA = balls[i]->transform->position;

		//iterate through all balls that haven't been checked
		for (size_t j = i + 1; j < 15; ++j) {

			//skip falling ball
			if (balls[j]->isOff) continue;

			//position of the second ball
			glm::vec3 posB = balls[j]->transform->position;

			//collision test
			if (glm::length(posA - posB) <= ball_radius * 2.0f) {

				//get ball velocity
				glm::vec3 velA = balls[i]->velocity;
				glm::vec3 velB = balls[j]->velocity;

				//calculate hit position
				glm::vec3 diff_p = posA - posB;
				glm::vec3 diff_v = velA - velB;
				float A = glm::dot(diff_v, diff_v);
				float B = -2.0f * glm::dot(diff_p, diff_v);
				float C = glm::dot(diff_p, diff_p) - 4.0f * ball_radius * ball_radius;
				float hit_t = solve_quadratic_equation(A, B, C);

				//traverse back in time
				posA -= velA * hit_t;
				posB -= velB * hit_t;

				//reset ball position
				balls[i]->transform->position = posA;
				balls[j]->transform->position = posB;

				//hit direction
				glm::vec3 hit_normal = normalize(posA - posB);

				//Seperate ball velocity direction
				glm::vec3 velA_normal = glm::dot(velA, hit_normal) * hit_normal;
				glm::vec3 velA_tangent = velA - velA_normal;
				glm::vec3 velB_normal = glm::dot(velB, hit_normal) * hit_normal;
				glm::vec3 velB_tangent = velB - velB_normal;

				//exchange velocity
				balls[i]->velocity = velB_normal * ball_ball_collision + velA_tangent;
				balls[j]->velocity = velA_normal * ball_ball_collision + velB_tangent;
			}
		}
	}

	//------------- update balls' transform ------------------
	
	//friction
	float friction = 0.2f;

	for (size_t index = 0; index < 15; ++index) {

		//skip falling ball
		if (balls[index]->isOff) continue;

		//friction
		if (balls[index]->velocity != glm::vec3(0.0f)) {
			glm::vec3 velocity_prev = balls[index]->velocity;
			balls[index]->velocity += glm::normalize(-balls[index]->velocity) * friction * elapsed;
			if (glm::dot(velocity_prev, balls[index]->velocity) < 0.0f) {
				balls[index]->velocity = glm::vec3(0.0f);
			}
		}
		
		//update position
		balls[index]->transform->position += balls[index]->velocity * elapsed;

		//update rotation
		if (balls[index]->velocity != glm::vec3(0.0f)) {
			glm::vec3 axis = glm::normalize(glm::cross(balls[index]->velocity, glm::vec3(0.0f, 0.0f, -1.0f)));
			float angle = glm::length(balls[index]->velocity * elapsed) / ball_radius;
			balls[index]->transform->rotation = glm::angleAxis(angle, axis) * balls[index]->transform->rotation;
		}
	}
	

	//reset button press counters:
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;

}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//update camera aspect ratio for drawable:
	camera->aspect = float(drawable_size.x) / float(drawable_size.y);

	//set up light type and position for lit_color_texture_program:
	// TODO: consider using the Light(s) in the scene to do this
	glUseProgram(lit_color_texture_program->program);
	glUniform1i(lit_color_texture_program->LIGHT_TYPE_int, 1);
	glUniform3fv(lit_color_texture_program->LIGHT_DIRECTION_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f,-1.0f)));
	glUniform3fv(lit_color_texture_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 0.95f)));
	glUseProgram(0);

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClearDepth(1.0f); //1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); //this is the default depth comparison function, but FYI you can change it.

	GL_ERRORS(); //print any errors produced by this setup code

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

		//draw hints
		float H = 0.08f;
		lines.draw_text("WASD to move and hit all other balls into the holes",
			glm::vec3(-aspect + 0.1f * H, -1.0 + 0.1f * H, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0x00, 0x00, 0x00, 0x00));
		float ofs = 2.0f / drawable_size.y;
		lines.draw_text("WASD to move and hit all other balls into the holes",
			glm::vec3(-aspect + 0.1f * H + ofs, -1.0 + + 0.1f * H + ofs, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0xff, 0xff, 0xff, 0x00));

		//draw records
		lines.draw_text(Recordtext,
			glm::vec3(-aspect + 0.1f * H, 0.85 + 0.1f * H, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0x00, 0x00, 0x00, 0x00));
		lines.draw_text(Recordtext,
			glm::vec3(-aspect + 0.1f * H + ofs, 0.85 + 0.1f * H + ofs, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0xff, 0xff, 0xff, 0x00));
	}
}
