#ifndef RIGID_BODY_H
#define RIGID_BODY_H

#include "vm.h"

typedef struct rigid_body
{
    v3 position;        /* World-space position of the body's origin (usually center of mass) */
    v3 velocity;        /* Linear velocity of the center of mass (in m/s) */
    v3 force;           /* Accumulated force to apply this frame (reset after integration) */
    v3 torque;          /* Accumulated torque to apply this frame (reset after integration) */
    v3 angularVelocity; /* Angular velocity in world space (radians/sec) */
    float mass;         /* Total mass of the body (in kilograms) */
    float inertia;      /* Scalar rotational inertia (resistance to rotation) */
    quat orientation;   /* Current rotation of the body as a quaternion */

} rigid_body;

rigid_body rigid_body_init(v3 position, quat orientation, float mass, float inertia)
{
    rigid_body result;

    result.position = position;
    result.orientation = orientation;
    result.mass = mass;
    result.inertia = inertia;
    result.velocity = vm_v3_zero;
    result.angularVelocity = vm_v3_zero;
    result.force = vm_v3_zero;
    result.torque = vm_v3_zero;

    return (result);
}

v3 rigid_body_forward(rigid_body *rb)
{
    return vm_quat_forward(rb->orientation);
}
v3 rigid_body_right(rigid_body *rb)
{
    return vm_quat_right(rb->orientation);
}
v3 rigid_body_up(rigid_body *rb)
{
    return vm_quat_up(rb->orientation);
}

v3 rigid_body_point_velocity(rigid_body *rb, v3 worldPoint)
{
    v3 r = vm_v3_sub(worldPoint, rb->position);
    v3 rotational = vm_v3_cross(rb->angularVelocity, r);
    return vm_v3_add(rb->velocity, rotational);
}

void rigid_body_apply_force_at_position(rigid_body *rb, v3 force, v3 position)
{
    v3 r = vm_v3_sub(position, rb->position);

    rb->force = vm_v3_add(rb->force, force);
    rb->torque = vm_v3_add(rb->torque, vm_v3_cross(r, force));
}

void rigid_body_integrate(rigid_body *rb, float dt)
{
    float invMass = 1.0f / rb->mass;
    float invInertia = 1.0f / rb->inertia;

    v3 acceleration = vm_v3_mulf(rb->force, invMass);
    v3 angularAcceleration;
    float angle;

    rb->velocity = vm_v3_add(rb->velocity, vm_v3_mulf(acceleration, dt));
    rb->position = vm_v3_add(rb->position, vm_v3_mulf(rb->velocity, dt));

    angularAcceleration = vm_v3_mulf(rb->torque, invInertia);
    rb->angularVelocity = vm_v3_add(rb->angularVelocity, vm_v3_mulf(angularAcceleration, dt));

    angle = vm_v3_length(rb->angularVelocity) * dt;

    if (angle > 0.0001f)
    {
        v3 axis = vm_v3_normalize(rb->angularVelocity);
        quat dq = vm_quat_rotate(axis, angle);
        rb->orientation = vm_quat_normalize(vm_quat_mul(dq, rb->orientation));
    }

    rb->force = vm_v3_zero;
    rb->torque = vm_v3_zero;
}

#endif /* RIGID_BODY_H */