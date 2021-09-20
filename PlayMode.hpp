#include "Mode.hpp"

#include "Scene.hpp"

#include <glm/glm.hpp>

#include <string>
#include <vector>
#include <deque>
#include <cmath>

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

	//ball struct
	struct Ball {
		Scene::Transform* transform = nullptr;
		glm::quat base_rotation;
		glm::vec3 base_location;
		glm::vec3 velocity = glm::vec3(0.0f, 0.0f, 0.0f);
		//glm::vec3 accel = glm::vec3(0.0f, 0.0f, 0.0f);
		bool isOff = false;

		Ball(Scene::Transform* transform){
			isOff = false;
			//accel = glm::vec3(0.0f, 0.0f, 0.0f);
			velocity = glm::vec3(0.0f, 0.0f, 0.0f);
			this->transform = transform;
			base_rotation = transform->rotation;
			base_location = transform->position;
		}

		void reset() {
			transform->rotation = base_rotation;
			transform->position = base_location;
			isOff = false;
			velocity = glm::vec3(0.0f, 0.0f, 0.0f);
			//accel = glm::vec3(0.0f, 0.0f, 0.0f);
		}
	};

	Ball * balls[15];

	//game status
	float current_time = 0.0f;
	float best_time = 1000000.0f;
	bool has_best = false;
	int num_balls = 15;
	bool isGameOver = false;

	std::string Recordtext;
	
	//camera:
	Scene::Camera *camera = nullptr;

};
