// ******************************************************
// Project Name    : ForRocket
// File Name       : rocket.cpp
// Creation Date   : 2019/10/20

// Copyright © 2019 Susumu Tanaka. All rights reserved.
// ******************************************************

#include "rocket.hpp"

#include <cmath>

#ifdef DEBUG
#include <iostream>
#endif

forrocket::Rocket::Rocket() {
    length_thrust = 0.0;
    diameter = 0.0;
    area = 0.0;
    length = 0.0;
    inertia_tensor << 0.0, 0.0, 0.0,
                        0.0, 0.0, 0.0,
                        0.0, 0.0, 0.0;

    cant_angle_fin = 0.0;

    dynamic_pressure = 0.0;

    quaternion_dot << 0.0, 0.0, 0.0, 0.0;
    angular_velocity << 0.0, 0.0, 0.0;
    angular_acceleration << 0.0, 0.0, 0.0;
    angle_of_attack = 0.0;
    sideslip_angle = 0.0;

};

// Parameter Setter //////////////////
////////////////////////////////////////////////////////

// void forrocket::Rocket::setEngine(Engine engine) {
//     this->engine = engine;
// };

void forrocket::Rocket::setLengthCG(const InterpolateParameter length_CG) {
    this->length_CG_src = length_CG;
};

void forrocket::Rocket::setLengthCP(const InterpolateParameter length_CP) {
    this->length_CP_src = length_CP;
};

void forrocket::Rocket::setCA(const InterpolateParameter CA) {
    this->CA_src = CA;
};

void forrocket::Rocket::setCA(const InterpolateParameter CA, const InterpolateParameter CA_burnout) {
    this->CA_src = CA;
    this->CA_burnout_src = CA_burnout;
};

void forrocket::Rocket::setCNa(const InterpolateParameter CNa) {
    this->CNa_src = CNa;
};

void forrocket::Rocket::setCld(const InterpolateParameter Cld) {
    this->Cld_src = Cld;
};

void forrocket::Rocket::setClp(const InterpolateParameter Clp) {
    this->Clp_src = Clp;
};

void forrocket::Rocket::setCmq(const InterpolateParameter Cmq) {
    this->Cmq_src = Cmq;
};

void forrocket::Rocket::setCnr(const InterpolateParameter Cnr) {
    this->Cnr_src = Cnr;
};

void forrocket::Rocket::setInertiaTensor(const InterpolateParameter MOI_xx, const InterpolateParameter MOI_yy, const InterpolateParameter MOI_zz) {
    this->inertia_moment_xx_src = MOI_xx;
    this->inertia_moment_yy_src = MOI_yy;
    this->inertia_moment_zz_src = MOI_zz;
};

void forrocket::Rocket::setAttitudeProgram(const InterpolateParameter azimuth, const InterpolateParameter elevation, const InterpolateParameter roll) {
    this->azimuth_program_src = azimuth;
    this->elevation_program_src = elevation;
    this->roll_program_src = roll;
};

void forrocket::Rocket::setCdSParachute(const double CdS_first) {
    CdS_parachute_src.push_back(CdS_first);
};

void forrocket::Rocket::setCdSParachute(const double CdS_first, const double CdS_second) {
    CdS_parachute_src.push_back(CdS_first);
    CdS_parachute_src.push_back(CdS_second);
};




// Parameter Getter //////////////////
////////////////////////////////////////////////////////
double forrocket::Rocket::getLengthCG() {
    if (engine.burning) {
        this->length_CG = length_CG_src(burn_clock.countup_time);
    }
        
    return this->length_CG;
};

double forrocket::Rocket::getLengthCP(const double mach_number) {
    this->length_CP = length_CP_src(mach_number);
    return this->length_CP;
};

double forrocket::Rocket::getCA(const double mach_number) {
    if (engine.burning) {
        this->CA = CA_src(mach_number);
    } else {
        this->CA = CA_burnout_src(mach_number);
    }
    return this->CA;
};

double forrocket::Rocket::getCNa(const double mach_number) {
    this->CNa = CNa_src(mach_number);
    return this->CNa;
};

double forrocket::Rocket::getCld(const double mach_number) {
    this->Cld = Cld_src(mach_number);
    return this->Cld;
};

double forrocket::Rocket::getClp(const double mach_number) {
    this->Clp = Clp_src(mach_number);
    if (this->Clp > 0.0) {
        this->Clp *= -1.0;
    }
    return this->Clp;
};

double forrocket::Rocket::getCmq(const double mach_number) {
    this->Cmq = Cmq_src(mach_number);
    if (this->Cmq > 0.0) {
        this->Cmq *= -1.0;
    }
    return this->Cmq;
};

double forrocket::Rocket::getCnr(const double mach_number) {
    this->Cnr = Cnr_src(mach_number);
    if (this->Cnr > 0.0) {
        this->Cnr *= -1.0;
    }
    return this->Cnr;
};


Eigen::Vector3d forrocket::Rocket::getThrust(const double air_pressure) {
    engine.Update(burn_clock.countup_time, air_pressure, mass.propellant);

    Eigen::Vector3d thrust;
    if (engine.burning) {
        Eigen::Vector3d gimbal_angle(std::cos(engine.gimbal_angle_y_axis) * std::cos(engine.gimbal_angle_y_axis),
                                    std::sin(engine.gimbal_angle_z_axis),
                                    -std::sin(engine.gimbal_angle_y_axis));
        thrust << engine.thrust * gimbal_angle.array();
    } else {
        thrust << 0.0, 0.0, 0.0;
    }

    return thrust;
};

Eigen::Matrix3d forrocket::Rocket::getInertiaTensor() {
    if (engine.burning) {
        Eigen::Matrix3d tensor;
        double t = burn_clock.countup_time;
        tensor << inertia_moment_xx_src(t), 0.0, 0.0,
                0.0, inertia_moment_yy_src(t), 0.0,
                0.0, 0.0, inertia_moment_zz_src(t);
        this->inertia_tensor = tensor;
    }
    return this->inertia_tensor;
};

Eigen::Vector3d forrocket::Rocket::getAttitude() {
    Eigen::Vector3d attitude;
    double t = burn_clock.countup_time;
    attitude << azimuth_program_src(t), elevation_program_src(t), roll_program_src(t);
    return attitude;
};


// SOE Handler
void forrocket::Rocket::IgnitionEngine(DateTime UTC_init, double countup_time_init) {
    burn_clock = SequenceClock(UTC_init, countup_time_init);
    engine.Ignittion();
};

void forrocket::Rocket::CutoffEngine() {
    engine.Cutoff();
};

void forrocket::Rocket::DeSpin() {
    angular_acceleration[0] = 0.0;
    angular_velocity[0] = 0.0;
}


void forrocket::Rocket::JettsonFairing(const double mass_fairing) {
    mass.inert = mass.inert - mass_fairing;
    if (mass.inert <= 0.0) mass.inert = 1.0;
};

void forrocket::Rocket::SeparateUpperStage(const double mass_upper_stage) {
    mass.inert = mass.inert - mass_upper_stage;
    if (mass.inert <= 0.0) mass.inert = 1.0;
};

void forrocket::Rocket::OpenParachute() {
    if (count_open_parachute < CdS_parachute_src.size()) {
        CdS_parachute += CdS_parachute_src[count_open_parachute];
    }
    ++count_open_parachute;
};

