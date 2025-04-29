#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <json.hpp>
#include <cmath>
using json = nlohmann::json;
#include <SFML/Graphics.hpp>

const float distance_scalar = 100.f; // 1.0f/831111.0f;
const float radius_scalar_sun= 2.0f/ 34817.0f;
const float radius_scalar_planet = 2.0f/631.07;
const float G = 6.67E-2f;

struct Planet {
    std::string planet_name;
    float mass;
    float distance_from_sun_km;
    float planet_radius;
    float orbital_velocity;
    sf::Color color;
};

void from_json(const json& j, Planet& p){
    j.at("planet_name").get_to(p.planet_name);
    j.at("mass").get_to(p.mass);
    j.at("distance_from_sun_km").get_to(p.distance_from_sun_km);
    j.at("planet_radius").get_to(p.planet_radius);
    j.at("planet_name").get_to(p.planet_name);
    if(j.contains("orbital_velocity")){
        const auto& orbital_velocity = j.at("orbital_velocity");
        orbital_velocity.get_to(p.orbital_velocity);
    }
    else {
        p.orbital_velocity = 0;
    }
    if(j.contains("color")){
        const auto& color = j.at("color");
        p.color = sf::Color(
            color.at("r").get<uint8_t>(),
            color.at("g").get<uint8_t>(),
            color.at("b").get<uint8_t>(),
            color.value("a", 255) 

        );}
    else {
        p.color = sf::Color(255,255,255,255); //when no color has been given white
    }
}

class PlanetLoader {
    public:
    json load_from_file(const std::string& json_filename){
        return json::parse(std::ifstream(json_filename));
    }

    std::vector<Planet> parse_planets(const json& planet_data){
        return planet_data.get<std::vector<Planet>>();
    } 

};



class PlanetRenderer {

    public:
    std::string planet_name;
    sf::Vector2f velocity;
    sf::Vector2f position;
    sf::CircleShape shape;

        PlanetRenderer(
            sf::Vector2f drawn_position, float drawn_radius, sf::Color color,
            float mass, sf::Vector2f position, sf::Vector2f velocity = sf::Vector2f(0, 0), std::string planet_name = "")
            : position(position), mass(mass), velocity(velocity), inverse_mass(1.f/mass), planet_name(planet_name)
        { 
            shape.setRadius(drawn_radius);
            shape.setPosition(drawn_position);
            shape.setFillColor(color);
            shape.setOrigin(drawn_radius, drawn_radius); 
        }

        void planet_render(sf::RenderWindow& window){
            window.draw(shape);
        }
        void update_position(float dt){
            position += velocity * dt ;
            sf::Vector2f drawn_position = position * distance_scalar;
            shape.setPosition(drawn_position);
        }

        void update_velocity(const std::vector<PlanetRenderer>& planets, float dt) {
            sf::Vector2f total_force = sf::Vector2f(0.f, 0.f);

            // Debug: Print current planet's mass and inverse mass
            std::cout << "\n--- Updating velocity for planet ---\n";
            std::cout << "Mass: " << mass << " kg\n";
            std::cout << "Inverse Mass: " << inverse_mass << " kg⁻¹\n";
            std::cout << "Position: (" << position.x << ", " << position.y << ") km\n";

            for (const auto& other : planets) {
                if (&other == this) continue;

                // Debug: Print other planet's details
                std::cout << "  Other Mass: " << other.mass << " kg\n";
                std::cout << "  Other Position: (" << other.position.x << ", " 
                        << other.position.y << ") km\n";

                // Calculate distance between planets
                sf::Vector2f distance_delta = other.position - position;
                float square_distance = distance_delta.x * distance_delta.x 
                                    + distance_delta.y * distance_delta.y;

                // Debug: Print distance calculations
                std::cout << "  Δ Distance: (" << distance_delta.x << ", " 
                        << distance_delta.y << ") km\n";
                std::cout << "  Square Distance: " << square_distance << " km²\n";

                if (square_distance == 0) {
                    std::cout << "  !!! Zero distance detected - skipping !!!\n";
                    continue;
                }

                // Calculate direction
                float inv_length = 1.f / std::sqrt(square_distance);
                sf::Vector2f direction = distance_delta * inv_length;

                // Debug: Print direction and intermediate values
                std::cout << "  Inv Length: " << inv_length << " km⁻¹\n";
                std::cout << "  Direction: (" << direction.x << ", " 
                        << direction.y << ")\n";

                // Calculate gravitational force
                float force_magnitude = G * (mass * other.mass) / square_distance;
                sf::Vector2f force = force_magnitude * direction;

                // Debug: Print force contribution
                std::cout << "  Force Magnitude: " << force_magnitude << " N\n";
                std::cout << "  Force Vector: (" << force.x << ", " 
                        << force.y << ") N\n";

                total_force += force;
            }
            /*
            // Debug: Print total force and velocity before update
            std::cout << "\nTotal Force: (" << total_force.x << ", " 
                    << total_force.y << ") N\n";
            std::cout << "Velocity Before: (" << velocity.x << ", " 
                    << velocity.y << ") km/s\n";

            velocity += total_force * inverse_mass * dt;

            // Debug: Print velocity after update
            std::cout << "Velocity After: (" << velocity.x << ", " 
                    << velocity.y << ") km/s\n";
            std::cout << "------------------------------\n";*/
        }
    

    private:
    float mass;
    float inverse_mass;

};

std::vector<PlanetRenderer> create_renderers(std::vector<Planet>& planet_data){
    
    float scaled_radius;
    
    std::vector<PlanetRenderer> renderers;
    
    for(const auto& p:planet_data){
        scaled_radius = p.planet_radius * (p.planet_name == "Sun" ? radius_scalar_sun: radius_scalar_planet);

        const float AU = 149.6e6f;       // 1 AU in km
        const float solar_mass = 1.989e30f; // 1 M☉ in kg
        const float years_per_sec = 1.0f / 3.154e7f; // Convert s → yr

        renderers.emplace_back(
            sf::Vector2f(0, p.distance_from_sun_km / AU * distance_scalar), // Screen position
            p.planet_radius * (p.planet_name == "Sun" ? radius_scalar_sun : radius_scalar_planet),
            p.color,
            p.mass / solar_mass,  // Mass in M☉
            sf::Vector2f(0, p.distance_from_sun_km / AU),  // Physics position (AU)
            sf::Vector2f(p.orbital_velocity * years_per_sec * AU, 0),  // Velocity (AU/yr)
            p.planet_name
        );
    }



    return renderers;
};


/*
class RenderPlanet
{
    sf::CircleShape planet;

    public:
    sf::Vector2f velocity;
    sf::Vector2f position;
    

        RenderPlanet(sf::Vector2f position, float mass, float radius, sf::Color color, sf::Vector2f initial_velocity = sf::Vector2f(0,0))
        {   
            this -> position = position;
            this -> mass = mass;
            this -> inverse_mass = 1 / mass;
            this -> velocity = initial_velocity;

            planet.setPosition(position);
            planet.setRadius(radius);
            planet.setFillColor(color);
        }

        void render(sf::RenderWindow& window)
        {
            window.draw(planet);
        }

        void updatePosition(float dt)
        {
            position += velocity * dt;
            planet.setPosition(position);
            

        }

        const float G = 5; //supposed to be the gravitational constant, now for testing purposes value is different (100)

        void updateVelocity(RenderPlanet& planet1, RenderPlanet& planet2, float dt )
        {   
            sf::Vector2f delta_distance = planet1.position - planet2.position;
            float distance = std::sqrt((delta_distance.x * delta_distance.x) + (delta_distance.y * delta_distance.y));
            sf::Vector2f direction = delta_distance / distance ;

            float F_grav_magn = G * (planet1.mass * planet2.mass) / (distance * distance); 
            sf::Vector2f Force = F_grav_magn * direction;
        
            planet1.velocity -= sf::Vector2f((Force.x * planet1.inverse_mass ), (Force.y * planet1.inverse_mass)) * dt;
            planet2.velocity += sf::Vector2f((Force.x * planet2.inverse_mass ), (Force.y * planet2.inverse_mass)) * dt;


            
        }

    private:
        float mass;
        float inverse_mass;
        
}; */









int main()
{
    sf::RenderWindow window(sf::VideoMode(1280, 720), "orbit simulator", sf::Style::None);
    window.setFramerateLimit(1);
    sf::View view(sf::FloatRect(-640, -360, 1280, 720));
    window.setView(view);

    PlanetLoader loader;
    std::vector<Planet> json_planets = loader.parse_planets(loader.load_from_file("planets.json"));
/*
// Debug: Print parsed JSON data
    std::cout << "\n=== Parsed Planet Data ===\n";
    for (const auto& p : json_planets) {
        std::cout << "Name: " << p.planet_name << "\n"
                << "  Mass: " << p.mass << " kg\n"
                << "  Distance: " << p.distance_from_sun_km << " km\n"
                << "  Radius: " << p.planet_radius << " km\n"
                << "  Velocity: " << p.orbital_velocity << " km/s\n"
                << "  Color: (" << static_cast<int>(p.color.r) << ", "
                                << static_cast<int>(p.color.g) << ", "
                                << static_cast<int>(p.color.b) << ")\n"; 
    }
*/
    
    std::vector<PlanetRenderer> planet_data = create_renderers(json_planets);
/*
    // Debug: Print renderer initialization
    std::cout << "\n=== Planet Renderer Initialization ===\n";
    for (const auto& pr : planet_data) {
        std::cout << "Renderer for: " << "\n"
                << "  Scaled radius: " << pr.shape.getRadius() << " px\n"
                << "  Physics position: (" << pr.position.x << ", " 
                                            << pr.position.y << ") km\n"
                << "  Drawn position: (" << pr.shape.getPosition().x << ", "
                                        << pr.shape.getPosition().y << ") px\n"
                << "  Initial velocity: (" << pr.velocity.x << ", "
                                            << pr.velocity.y << ") km/s\n";
    }
*/




    float dt = 1.f/365.f; 

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed) window.close();
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) window.close();
        }


        for(auto& planet : planet_data) {
            planet.update_velocity(planet_data, dt);
            planet.update_position(dt);
        }

        // Rendering
        window.clear(sf::Color::Black);
        for(auto& planet : planet_data) {
            planet.planet_render(window);
        }
        window.display();
    }
 

 
 
    return 0;
}