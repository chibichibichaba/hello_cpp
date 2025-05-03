d_earth_t_sun = 149600000
r_earth = 6371
r_sun = 696340

AU_KM = 1/(149.6E6)
mass_scalar = 1/1.989E30

years_sec = 1/(3.15E7)

G = 39.40


print(f"Distance scalar: {600/(d_earth_t_sun*AU_KM)}\n\n") 
print(f"Radius scalar for the earth: {15/(r_earth)}\n\n") 
print(f"Radius scalar for the sun: {30/(r_sun)}\n\n")
print(f"Velocity scalar: {AU_KM/years_sec}")  