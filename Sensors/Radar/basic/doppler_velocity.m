% Doppler Velocity Calculation
c = 3*10^8;         %speed of light
frequency = 77e9;   %frequency in Hz

% TODO : Calculate the wavelength
lamda = c/frequency;

% TODO : Define the doppler shifts in Hz using the information from above 
dopplerShift = [3 * 10^3, -4.5 * 10^3, 11 * 10^3, -3 *10^3];

% TODO : Calculate the velocity of the targets  fd = 2*vr/lambda
Vr = dopplerShift * lamda / 2;

% TODO: Display results
disp(Vr);