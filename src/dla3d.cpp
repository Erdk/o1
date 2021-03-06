#include <iostream>
#include <iomanip>
#include <vector>
#include <cstdlib>
#include <cassert>
#include <fstream>

#include <boost/program_options.hpp>

namespace po = boost::program_options;

// lattice dimmensions
int width;
int height;
int depth;

// target cluster size
int cl_size;

// controls verbosity
bool verbose;

bool progress;

class Particle {
	public:
		// position on lattice
		int x, y, z;

		Particle(int x_, int y_, int z_) {
			x = x_;
			y = y_;
      z = z_;
		}

		// Brownian random walk
		void randomWalk() {
      // up(0):   y++
      // down(1): y--
      // left(2): x--
      // right(3): x++
      // near(4): z--
      // far(5): z++
			switch(rand() % 6) {
				case 0:
					y++;
					break;
				case 1:
					y--;
					break;
				case 2:
					x--;
					break;
				case 3:
					x++;
					break;
				case 4:
					z--;
					break;
				case 5:
					z++;
					break;
			}

			if (x < 0 || x > width || y < 0 || y > height || z < 0 || z > depth) {
				x = rand() % width;
				y = rand() % height;
        z = rand() % depth;
			}
		}
};

class Cluster {
	public:
		std::vector<Particle*> particles;
		Cluster() {
			particles = std::vector<Particle*>();
			particles.push_back(new Particle(width/2, height/2, depth/2));
		}

		virtual ~Cluster() {
			for (Particle* p : particles) {
				delete p;
			}
		}

		void checkClusterParticleCollision(Particle** p) {
			bool collide = checkCollision(*p);
			if (collide) {
				particles.push_back(*p);

				*p = new Particle(rand() % width, rand() % height, rand() % depth);

        if (progress) {
          std::cerr << particles.size() << " ";
        }
			}
		}

		bool checkCollision(Particle* p) {
			for (Particle* i : particles) {
				if ((p->x == i->x - 1 || p->x == i->x || p->x == i->x + 1) &&
						(p->y == i->y - 1 || p->y == i->y || p->y == i->y + 1) &&
						(p->z == i->z - 1 || p->z == i->z || p->z == i->z + 1)) {
					return true;
				}
			}

			return false;
		}
};

int main(int argc, const char* argv[]) {

	// <-- handle options -->
	po::options_description desc("Allowed options");
	desc.add_options()
		("help,h", "print this help message")
    ("width,w", po::value<int>(&width)->default_value(900), "width of lattice") 
		("height,e", po::value<int>(&height)->default_value(900), "height of lattice")
		("depth,d", po::value<int>(&depth)->default_value(900), "depth of lattice")
		("cluster-size,n", po::value<int>(&cl_size)->default_value(1000), "target size of cluster")
		("progress,p", "show progress")
    ("verbose,v", "verbose output")
		;

	po::variables_map vm;
  try{
	  po::store(po::parse_command_line(argc, argv, desc), vm);
  } catch (std::exception const& exc) {
    // handle unrecognized option
    std::cerr << exc.what() << std::endl << std::endl;
    std::cerr << desc << std::endl;
    return 1;
  }
	po::notify(vm);

  // print help
  if (vm.count("help")) {
    std::cout << desc << std::endl;
    return 1;
  }

  // turn on progress
  if (vm.count("progress")) {
    progress = true;
  } else {
    progress = false;
  }

  // turn on verbosity
  if (vm.count("verbose")) {
    verbose = true;
  } else {
    verbose = false;
  }

  // <-- handle options -->

  // print parameters of simulation
  if (verbose) {
    std::cout << "w: " << width << " h: " << height << " d: " << depth << " cl: " << cl_size << " progress: " << progress << std::endl;
  }

	// cluster with one point in the center of lattice
	Cluster c = Cluster();

	// first "walking particle"
	Particle* walker = new Particle(rand() % width, rand() % height, rand() % depth);

	// run until there's 'cl_size' particles in cluster, part of them will be
	// duplicated, but observed loss was about ~10%, which is more acceptable
	// thanadditional time of finding new random and non-colliding place to spawn
	while(c.particles.size() < cl_size) {

    // move particle in random direction on lattice
		walker->randomWalk();

    // check if particle collided with any of particles from cluster
		c.checkClusterParticleCollision(&walker);
	}

  std::ofstream output;
  output.open("result.csv");
	for (Particle* p : c.particles) {
		output << p->x << "," << p->y << "," << p->z << "\n";
	}
  output.close();

	return 0;
}
