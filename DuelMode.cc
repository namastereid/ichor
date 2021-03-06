#include "GameMode.h"
#include "Player.h"
#include "Explosion.h"
#include "Particle.h"
#include "util.h"
#include "Input.h"

class DuelMode : public GameMode
{
private:
    Player ppos_;
    Player pneg_;

    FluidDensityGrid* field_;

    std::list<Explosion*> explosions_;

    std::list<Particle> particles_;
    float particles_pending_;

    Input* posinput_;
    Input* neginput_;
    
    void check_hit() {
        if ((ppos_.get_life() <= 0 && ppos_.vulnerable())
         || (pneg_.get_life() <= 0 && pneg_.vulnerable())) {
            QUIT = true;
        }

        if (ppos_.vulnerable() && pneg_.vulnerable()) {
            if (ppos_.check_hit(field_)) {
                ppos_.die();
                float amt = field_->get_balance();
                ppos_.store(amt > 0 ? 0 : -DIE_ENERGY_FACTOR * amt);
                explosions_.push_back(new InwardVortexExplosion(ppos_.get_color(), ppos_.position(), DEATH_FLUID, INVINCIBLE_TIME));
            }
            if (pneg_.check_hit(field_)) {
                pneg_.die();
                float amt = field_->get_balance();
                pneg_.store(amt < 0 ? 0 : DIE_ENERGY_FACTOR * amt);
                explosions_.push_back(new InwardVortexExplosion(pneg_.get_color(), pneg_.position(), -DEATH_FLUID, INVINCIBLE_TIME));
            }
        }
    }

    void frame_input() {
        posinput_->step();
        neginput_->step();
        ppos_.set_blow(posinput_->get_direction());
        pneg_.set_blow(neginput_->get_direction());
    }

    void clear_field() {
        delete field_;
        field_ = new FluidDensityGrid(W,H,vec(0,0), vec(W,H), DIFFUSION, VISCOSITY);
    }

    void reset() {
        clear_field();
        ppos_ = Player(Color(1, 0.5, 0), 1, vec(CLAMPW, H-CLAMPH-1), ppos_.get_life(), load_texture(ICHOR_DATADIR "/redfirefly.png"));
        pneg_ = Player(Color(0, 0.5, 1), -1, vec(W-CLAMPW-1, CLAMPH), pneg_.get_life(), load_texture(ICHOR_DATADIR "/bluefirefly.png"));

        for (std::list<Explosion*>::iterator i = explosions_.begin(); 
             i != explosions_.end(); ++i) {
            delete *i;
        }
        explosions_.empty();
    }

    void check_unstable() {
        float density = field_->get_density_direct(W/2,H/2);
        if (!(density > -10 && density < 10)) {
            reset();
        }
    }

    void check_particles() {
        for (std::list<Particle>::iterator i = particles_.begin(); i != particles_.end();) {
            i->step(field_);
            
            bool eaten = false;
            if ((i->pos - ppos_.position()).norm2() < EATDIST*EATDIST) {
                ppos_.store(EATENERGY);
                eaten = true;
            }
            if ((i->pos - pneg_.position()).norm2() < EATDIST*EATDIST) {
                pneg_.store(EATENERGY);
                eaten = true;
            }
            if (i->pos.x < CLAMPW || i->pos.x > W-CLAMPW || i->pos.y < CLAMPH || i->pos.y > H-CLAMPH) {
                eaten = true;
            }

            if (!eaten) {
                ++i;
            }
            else {
                std::list<Particle>::iterator iprime = i;
                ++iprime;
                particles_.erase(i);
                i = iprime;
            }
        }

        particles_pending_ += PARTICLE_RATE * DT;
        while (particles_pending_ > 1) {
            particles_pending_ -= 1;
            vec pos(randrange(2,W-3), randrange(2, H-3));
            field_->set_density(pos, field_->get_density(pos) / 1.2);
            particles_.push_back(Particle(vec(randrange(2,W-3), randrange(2,H-3))));
        }
    }
    
public:
    DuelMode(Input* posinput, Input* neginput) 
        : ppos_(Color(1, 0.5, 0), 1, vec(CLAMPW, H-CLAMPH-1), 5, load_texture(ICHOR_DATADIR "/redfirefly.png")),
          pneg_(Color(0, 0.5, 1), -1,vec(W-CLAMPW-1, CLAMPH), 5, load_texture(ICHOR_DATADIR "/bluefirefly.png")),
          field_(0), particles_pending_(0),
          posinput_(posinput), neginput_(neginput)
    {
        clear_field();
    }

    ~DuelMode() {
        delete posinput_;
        delete neginput_;
        delete field_;
        for (std::list<Explosion*>::iterator i = explosions_.begin();
             i != explosions_.end(); ++i) {
            delete *i;
        }
        explosions_.empty();
    }
    
    void step() {
        check_unstable();
        frame_input();
        
        check_hit();

        for (std::list<Explosion*>::iterator i = explosions_.begin();
             i != explosions_.end();) {
            if ((*i)->done()) {
                delete *i;
                std::list<Explosion*>::iterator tmp = i;
                ++tmp;
                explosions_.erase(i);
                i = tmp;
            }
            else {
                (*i)->step(field_);
                ++i;
            }
        }
        
        field_->step_velocity();
        field_->step_density();

        ppos_.step(field_);
        pneg_.step(field_);

        check_particles();
    }

    void draw() const {
        draw_board(field_, W, H);

        ppos_.draw();
        pneg_.draw();
        ppos_.draw_lives(vec(3,H-3), 1);
        pneg_.draw_lives(vec(W-4,H-3), -1);

        draw_particles(particles_);
        
        for (std::list<Explosion*>::const_iterator i = explosions_.begin();
             i != explosions_.end(); ++i) {
            (*i)->draw();
        }
    }

    bool events(SDL_Event* e) {
        return posinput_->events(e) || neginput_->events(e);
    }
};

GameMode* make_DuelMode(Input* pos, Input* neg) {
    return new DuelMode(pos, neg);
}
