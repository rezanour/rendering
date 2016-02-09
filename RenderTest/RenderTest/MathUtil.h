#pragma once

// Given a desired direction to point (and an up vector), compute a quaternion representing that orientation.
XMVECTOR __vectorcall QuaternionFromViewDirection(FXMVECTOR forward, FXMVECTOR up);
