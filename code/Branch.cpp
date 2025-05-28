#include "Branch.h"
#include <string>    // loading from a string
#include <vector>   
#include <sstream>  
#include <cmath>     // for branch calcuslations
#include <algorithm> 
#include <cstdlib>   

 /** Dictionary
  * 
  *  branchIndex - unique index of this branch
  *  parentBranchIndex - index of parent branch 
  *  initialAngle - angle relative to its parent
  *  initialLength - initial lenght
  *  initialWidth - same
  *  initialXPos - X coord
  *  initialYPos - Y coods
  *  
  * */ 

  // Starting up with a new branch... includes all of the above
Branch::Branch(int branchIndex, int parentBranchIndex, float initialAngle, float initialLength, float initialWidth, float initialXPos, float initialYPos): index(branchIndex), parentIndex(parentBranchIndex), age(0), turnsWithoutWater(0), turnsWithoutNutrients(0), isAlive(true) 
{
    cv::Size2f size = cv::Size2f(initialWidth, initialLength); 

    //finds centr of branch using base  (CHATGPT)
    float xPos = initialXPos+0.5*initialLength*sin(initialAngle * (M_PI / 180));
    float yPos = initialYPos-0.5*initialLength*cos(initialAngle * (M_PI / 180));

    Point centre = Point(xPos, yPos); //calc-ed center point stored

    branchRect = RotatedRect(centre, size, initialAngle);
}

// default const for branch
Branch::Branch() : Branch(-1, -1, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f) {
};

Branch::~Branch() {
    // Empty destructor - resources are managed by members (e.g., std::vector, RotatedRect)
}
float Branch::getAngle(){
    return branchRect.angle;
}
void Branch::getTipPos(float &xPosition, float &yPosition) {
    // Find the tips of branch trig (CHATGPT)
    xPosition = branchRect.center.x+0.5*branchRect.size.height*sin(branchRect.angle * (M_PI / 180));
    yPosition = branchRect.center.y-0.5*branchRect.size.height*cos(branchRect.angle * (M_PI / 180));
}

// -------------- life of bush

// increment counter to keep track of truns without water and fertliser
void Branch::incrementTurnsWithoutWater() {
    if (isAlive) {
        turnsWithoutWater++;
    }
}
void Branch::incrementTurnsWithoutNutrients() {
    if (isAlive) {
        turnsWithoutNutrients++;
    }
}

// checks how many turns since last supplied 
int Branch::getTurnsWithoutWater() const {
    return turnsWithoutWater;
}
int Branch::getTurnsWithoutNutrients() const {
    return turnsWithoutNutrients;
}

// resets when bush gets watered or fertilsier
void Branch::resetTurnsWithoutWater() {
    turnsWithoutWater = 0;
}
void Branch::resetTurnsWithoutNutrients() {
    turnsWithoutNutrients = 0;
}

// sets alive status of branch
void Branch::setIsAlive(bool aliveStatus) {
    isAlive = aliveStatus;
}

int Branch::getIndex(){
    return index;
}
int Branch::getParentIndex(){
    return parentIndex;
}
void Branch::addChild(int childIndex) { // add index to vectr
    childIndices.push_back(childIndex);
}
bool Branch::removeChild(int childIndex){ // loop for every child of branch
    for(int i = 0; i < childIndices.size(); i ++){
        //check if its being removed
        if(childIndices[i] == childIndex){
            // delete the elements
            childIndices.erase(childIndices.begin() + i);
            return true;
        }
    }
    //Returns false if the child is not found
    return false;
}


vector<int> Branch::getChildren(){
    return childIndices;
}

float Branch::getSize(){
    return branchRect.size.area();
}

void Branch::setPos(float newXPos, float newYPos){
    //sets center of branch based on cordinates
    branchRect.center.x = newXPos+0.5*branchRect.size.height*sin(branchRect.angle * (M_PI / 180));
    branchRect.center.y = newYPos-0.5*branchRect.size.height*cos(branchRect.angle * (M_PI / 180));
}

// growing the branches
void Branch::grow(float areaIncrease, float &widthIncrease, float &lengthIncrease){
    if (!isAlive) {
        widthIncrease = 0.0f;
        lengthIncrease = 0.0f;
        return;
    }
    //The change in length is equal to (n/age) times the change in width 
    //so the branch initially grows longer and then later grows wider
    
    const float n_factor = 20.0f;

    float current_width = branchRect.size.width;
    float current_length = branchRect.size.height;

    //Incrementage by one
    age++;

    //growth calculation here (CHATGPT)
    double discriminant = 0.0;
    if (age > 0) {
        discriminant = pow((n_factor * current_width) / age + current_length, 2) + (4 * n_factor * areaIncrease) / age;
    } else {
        discriminant = pow(current_length, 2); // length increase
    }

    if (discriminant < 0.0 || age == 0) { 
        widthIncrease = 0.0f;
        lengthIncrease = 0.0f;

    } else { //width increase if its old
        if (age > 0) { // Ensure age is positive for division
           widthIncrease = (-(n_factor * current_width) / age - current_length + sqrt(discriminant)) / (2 * n_factor / age);
        } else {
           widthIncrease = 0.0f; // dont grow if age is 0
        }

        if (widthIncrease < 0.0f) {
            widthIncrease = 0.0f; // positive set
        }
       
        if (age > 0) {
           lengthIncrease = (n_factor / age) * widthIncrease;
        } else {
           lengthIncrease = 0.0f; 
        }

        if (lengthIncrease < 0.0f) {
            lengthIncrease = 0.0f; 
        }
    }
    
    //applying the growth 
    branchRect.size.width += widthIncrease;
    branchRect.size.height += lengthIncrease;
}

//decrement age of branchs
void Branch::decrementAge(){
    if(age>0){
        age--;
    }
}

void Branch::modifySize(float widthChange, float lengthChange){
    // check size modifications?
    if(branchRect.size.width + widthChange <= 0 || branchRect.size.height + lengthChange <= 0) {
        cout << "Error in Branch.modifySize(), modifications to branch size not valid" << endl;
        return;
    }

    // changes the variables
    branchRect.size.width += widthChange;
    branchRect.size.height += lengthChange;
}

// --------------------------- visual implementation section

// drawing the bracnhes on screen

void Branch::draw(Mat* img){
    Point2f vertices2f[4];

    // get rectangle pouints
    branchRect.points(vertices2f);

    //vertices -> regular point objects
    vector<Point> vertices;
    for(int i = 0; i < 4; ++i){
        vertices.push_back(vertices2f[i]);
    }

    //Draws the branch to the image also color determineed based on age

    // RGB (CHATGPT)
    float baseR = 139.0f;
    float baseG = 69.0f;
    float baseB = 19.0f;

    const int maxAgeForColorEffect = 50; // older branches will be darker
    
    // age factor: from 0.0 (youngest) to 1.0 
    float ageFactor = 0.0f;
    if (maxAgeForColorEffect > 0) {
        ageFactor = static_cast<float>(std::min(this->age, maxAgeForColorEffect)) / static_cast<float>(maxAgeForColorEffect);
    }

    //brightness scales: 1.0 (original brightness) to 0.5 half
    float brightnessScale = 1.0f - (ageFactor * 0.5f); 

    // calc new colour valurs
    int r = static_cast<int>(baseR * brightnessScale);
    int g = static_cast<int>(baseG * brightnessScale);
    int b = static_cast<int>(baseB * brightnessScale);

    // must be in valid range or it will break
    r = std::max(0, std::min(255, r));
    g = std::max(0, std::min(255, g));
    b = std::max(0, std::min(255, b));

    if (!isAlive) {
        // colour dead branch with darker
        fillConvexPoly(*img, vertices, CV_RGB(101, 67, 33)); 
    } else {
        // draw living branch with age based color
        fillConvexPoly(*img, vertices, CV_RGB(r, g, b));
    }
}


// checks for if user is in the branch rectangle (for trimming) (CHATGPT for help because hard to find values if rectangles are not straight)
bool Branch::containsMouse(int mouseX, int mouseY){
    //point around centre of branch
    int newX = mouseX - branchRect.center.x;
    int newY = mouseY - branchRect.center.y;

    //Gets sin and cos of the angle (CHATGPT)
    float angleSin = sin(branchRect.angle*M_PI/180);
    float angleCos = cos(branchRect.angle*M_PI/180);

    //Rotates point
    int rotatedX = newX*angleCos - newY*angleSin;
    int rotatedY = newX*angleSin - newY*angleCos;

    //moves the point back to its previous position
    rotatedX += branchRect.center.x;
    rotatedY += branchRect.center.y;

    Point2f rectanglePoints[4];

    //Gets a non-rotated copy of the rectangle
    RotatedRect unrotatedRect = branchRect;
    unrotatedRect.angle = 0;
    unrotatedRect.points(rectanglePoints);

    //Creates unrotated rectangle with the same dimensions as the branch rectangle
    Rect newRect(rectanglePoints[1], rectanglePoints[3]);

    //Finds whether the rotated point is in the unrotated rectangle
    return newRect.contains(Point(rotatedX, rotatedY));
}

void Branch::printData(){
    cout << "Branch object" << endl;
    cout << "Index: " << index << endl;
    cout << "Parent index: " << parentIndex << endl;
    cout << "Age: " << age << endl;
    cout << "Is alive: " << (isAlive ? "Yes" : "No") << endl;
    cout << "Turns without water: " << turnsWithoutWater << endl;
    cout << "Turns without nutrients: " << turnsWithoutNutrients << endl;
    cout << "Position: (" << branchRect.center.x << ", " << branchRect.center.y << ")" << endl;
    cout << "Size: (Width: " << branchRect.size.width << ", Height: " << branchRect.size.height << ")" << endl;
    cout << "Angle: " << branchRect.angle << endl;
    cout << "Child indices: ";
    for(int i = 0; i < childIndices.size(); i++){
        cout << childIndices[i] << " ";
    }
    cout << endl;
}

// --------------------------- Saving and loading system (text file)
void Branch::saveToStream(std::ostream& out) const {
    out << "branch" //indicates that its our games file
        << " index " << index
        << " parent_index " << parentIndex
        << " age " << age
        << " center_x " << branchRect.center.x
        << " center_y " << branchRect.center.y
        << " width " << branchRect.size.width
        << " height " << branchRect.size.height
        << " angle " << branchRect.angle
        << " num_children " << childIndices.size();
    for (int childIdx : childIndices) {
        out << " " << childIdx; // child indices (space seprated)
    }
    // life data
    out << " turns_water " << turnsWithoutWater;         // Turns without water
    out << " turns_nutrients " << turnsWithoutNutrients; // tturns without nutrients
    out << " is_alive " << isAlive;                     // alive status
    out << std::endl;
}

// loading (first checks for first line to verify if its out games file)

//error handling first
Branch Branch::loadFromStream(std::istream& in) {
    std::string line;
    if (!std::getline(in, line)) {
        std::cerr << "Error: Could not read line for Branch." << std::endl;
        return Branch(); // Return default/invalid branch
    }

    std::istringstream iss(line);
    std::string K_BRANCH, K_INDEX, K_PARENT_INDEX, K_AGE, K_CENTER_X, K_CENTER_Y, K_WIDTH, K_HEIGHT, K_ANGLE, K_NUM_CHILDREN, K_TURNS_WATER, K_TURNS_NUTRIENTS, K_IS_ALIVE;
    
    int p_idx = -1, p_parent_index = -1, p_age = 0, p_num_children = 0, p_turns_water = 0, p_turns_nutrients = 0;
    float p_cx = 0.f, p_cy = 0.f, p_w = 0.f, p_h = 0.f, p_angle = 0.f;
    bool p_is_alive = false;
    std::vector<int> p_childIndices;

    iss >> K_BRANCH; // Read "branch" keyword

    if (K_BRANCH != "branch") {
        std::cerr << "Error: Failed to parse Branch data line. Using default." << std::endl;
        return Branch();
    }

    // Read all fields sequentially
    iss >> K_INDEX >> p_idx
        >> K_PARENT_INDEX >> p_parent_index
        >> K_AGE >> p_age
        >> K_CENTER_X >> p_cx 
        >> K_CENTER_Y >> p_cy 
        >> K_WIDTH >> p_w 
        >> K_HEIGHT >> p_h 
        >> K_ANGLE >> p_angle
        >> K_NUM_CHILDREN >> p_num_children;

    p_childIndices.resize(p_num_children);
    for (int i = 0; i < p_num_children; ++i) {
        iss >> p_childIndices[i];
    }

    // Continue reading sustenance data directly
    iss >> K_TURNS_WATER >> p_turns_water
        >> K_TURNS_NUTRIENTS >> p_turns_nutrients
        >> K_IS_ALIVE >> p_is_alive;

    // Final check after all reads from iss
    if (iss.fail() || 
        K_INDEX != "index" || K_PARENT_INDEX != "parent_index" || K_AGE != "age" ||
        K_CENTER_X != "center_x" || K_CENTER_Y != "center_y" || K_WIDTH != "width" || K_HEIGHT != "height" || K_ANGLE != "angle" ||
        K_NUM_CHILDREN != "num_children" || K_TURNS_WATER != "turns_water" || K_TURNS_NUTRIENTS != "turns_nutrients" || K_IS_ALIVE != "is_alive") {
        std::cerr << "Error: Failed to parse Branch data line. Using default." << std::endl;
        return Branch();
    }

    // Constructing the branch:
    // The Branch constructor takes initial position (base of branch), angle, length, width.
    // We have center_x, center_y, width, height, angle from file.
    // We need to calculate base_x, base_y for the constructor.
    // Base position calculation (inverse of how center is calculated in constructor from base):
    // float xPos = initialXPos+0.5*initialLength*sin(initialAngle * (M_PI / 180));
    // float yPos = initialYPos-0.5*initialLength*cos(initialAngle * (M_PI / 180));
    // So: initialXPos = xPos - 0.5*initialLength*sin(initialAngle * (M_PI / 180))
    //     initialYPos = yPos + 0.5*initialLength*cos(initialAngle * (M_PI / 180))
    
    float angle_rad_load = p_angle * (M_PI / 180.0f);
    float calculated_base_x = p_cx - (0.5f * p_h * std::sin(angle_rad_load));
    float calculated_base_y = p_cy + (0.5f * p_h * std::cos(angle_rad_load));

    Branch loadedBranch(p_idx, p_parent_index, p_angle, p_h, p_w, calculated_base_x, calculated_base_y);
    
    loadedBranch.age = p_age; 
    loadedBranch.childIndices = p_childIndices; // Assign child indices
    loadedBranch.turnsWithoutWater = p_turns_water;
    loadedBranch.turnsWithoutNutrients = p_turns_nutrients;
    loadedBranch.isAlive = p_is_alive;
    
    // The RotatedRect in loadedBranch is already set by its constructor based on base_x, base_y, angle, width, height.
    // We need to ensure its center matches p_cx, p_cy if the constructor logic for center calculation is complex.
    // Given the constructor takes base position, the loaded p_cx, p_cy were for the *center*.
    // The current Branch constructor calculates the center from the base.
    // So, providing the calculated_base_x, calculated_base_y should correctly set up the branchRect.
    // Let's verify branchRect's center after construction.
    // (Optional: add a check here if branchRect.center.x is close to p_cx and branchRect.center.y is close to p_cy)
    // For now, assume the constructor correctly sets the branchRect based on the provided base coordinates.

    return loadedBranch;
}
