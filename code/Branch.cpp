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
    // generating branch leaves
    hasLeaves = true; 
    leafPositions.clear(); 
    cv::Size2f size = cv::Size2f(initialWidth, initialLength); 

    //finds centr of branch using base  (CHATGPT)
    float xPos = initialXPos+0.5*initialLength*sin(initialAngle * (M_PI / 180));
    float yPos = initialYPos-0.5*initialLength*cos(initialAngle * (M_PI / 180));

    Point centre = Point(xPos, yPos); //calc-ed center point stored

    branchRect = RotatedRect(centre, size, initialAngle);
    generateLeaves(); // gengerates the small greeen leaves
}

// default const for branch
Branch::Branch() : Branch(-1, -1, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f) {
};
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
    if (!isAlive) {
        hasLeaves = false; // if not alive no leaves
        leafPositions.clear(); 
    }
}

// leaf generation method
void Branch::generateLeaves() {
    if (!isAlive) { // if dead, no leafs and clear new ones
        leafPositions.clear(); 
        hasLeaves = false; 
        return;
    }
    // make sure its less than max leafs
    if (!hasLeaves || leafPositions.size() >= MAX_LEAVES_PER_BRANCH) { 
        return;
    }

    // calcc how many new leaves to add
    // make sure age is >0
    int currentAge = std::max(0, age); 
    int leavesToAddPotential = (currentAge / 2) + 1;
    int newLeavesCount = std::min(MAX_LEAVES_PER_BRANCH - (int)leafPositions.size(), leavesToAddPotential);

    if (newLeavesCount <= 0) {
        return;
    }
        // this generates new leafs on the branches 
    for (int i = 0; i < newLeavesCount; ++i) {
        // Calculate base and tip coordinates of the branch
        float angle_rad = branchRect.angle * (float)(M_PI / 180.0);
        float half_len_sin_angle = 0.5f * branchRect.size.height * std::sin(angle_rad);
        float half_len_cos_angle = 0.5f * branchRect.size.height * std::cos(angle_rad);

        // tip of the branch 
        float tip_x = branchRect.center.x + half_len_sin_angle;
        float tip_y = branchRect.center.y - half_len_cos_angle;
        // base of the branch
        float base_x = branchRect.center.x - half_len_sin_angle;
        float base_y = branchRect.center.y + half_len_cos_angle;

        // random distance on branch centerlinee
        float distFactor = (float)rand() / RAND_MAX;
        // find leaf position 
        float leaf_on_line_x = base_x + distFactor * (tip_x - base_x);
        float leaf_on_line_y = base_y + distFactor * (tip_y - base_y);

        // perpendicular offset of branch
        // Offset can be 0.75 times the branch width on either side (width x 1.5 total range)
        float offsetFactor = ((float)rand() / RAND_MAX - 0.5f) * branchRect.size.width * 1.5f;

        // final leaf positionn
        // Perpendicular vector to (sin(angle_rad), -cos(angle_rad)) is (cos(angle_rad), sin(angle_rad)) (CHATGPT)
        float final_leaf_x = leaf_on_line_x + offsetFactor * std::cos(angle_rad);
        float final_leaf_y = leaf_on_line_y + offsetFactor * std::sin(angle_rad);
        
        leafPositions.push_back(cv::Point2f(final_leaf_x, final_leaf_y));
    }
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

    generateLeaves(); // generate new leafs after the turn
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

    // regen leaffs
    leafPositions.clear();
    generateLeaves();
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

        // draw leaves for bracnhes
        if (hasLeaves) { 
            for (const auto& leaf_pos : leafPositions) {
                cv::circle(*img, leaf_pos, 3, CV_RGB(0, 150, 0), -1); // small green circls
            }
        }
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
    // leaf data
    out << " has_leaves " << hasLeaves;
    out << " num_leaves " << leafPositions.size();
    for (const auto& pos : leafPositions) {
        out << " " << pos.x << " " << pos.y;
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
    std::string line; // line by line
    if (!std::getline(in, line)) {
        std::cerr << "Cant read line for Branch" << std::endl;
        return Branch(); // Return default/invalid branch
    }

    std::istringstream iss(line); //string stream to parse line
    std::string keyword;

    int p_idx = -1, p_parent_index = -1, p_age = 0; // values for age and indices
    float p_cx = 0.f, p_cy = 0.f, p_w = 0.f, p_h = 0.f, p_angle = 0.f; // values for branch rectangle
    int p_num_children = 0; //number of children on bush
    std::vector<int> p_childIndices;

    iss >> keyword; // "branch" 
    if (keyword != "branch") { 
        std::cerr << "Error: not valid file " << keyword << std::endl; //error
        return Branch(); 
    }

    // reading based on defined keywodsd (CHATGPT) errors
    iss >> keyword >> p_idx;             // index
    if (keyword != "index") { std::cerr << "Error expected 'index', got " << keyword << " for branch " << p_idx << std::endl; return Branch(); }
    
    iss >> keyword >> p_parent_index;    // parent_index
    if (keyword != "parent_index") { std::cerr << "Error expected 'parent_index', got " << keyword << " for branch " << p_idx << std::endl; return Branch(); }
    
    iss >> keyword >> p_age;             // age
    if (keyword != "age") { std::cerr << "Error expected 'age', got " << keyword << " for branch " << p_idx << std::endl; return Branch(); }

    iss >> keyword >> p_cx;              // center_x
    if (keyword != "center_x") { std::cerr << "Error expected'center_x', got " << keyword << " for branch " << p_idx << std::endl; return Branch(); }

    iss >> keyword >> p_cy;              // center_y
    if (keyword != "center_y") { std::cerr << "Error expected'center_y', got " << keyword << " for branch " << p_idx << std::endl; return Branch(); }

    iss >> keyword >> p_w;               // width
    if (keyword != "width") { std::cerr << "Error expected 'width', got " << keyword << " for branch " << p_idx << std::endl; return Branch(); }
    
    iss >> keyword >> p_h;               // height
    if (keyword != "height") { std::cerr << "Error expected 'height', got " << keyword << " for branch " << p_idx << std::endl; return Branch(); }
    
    iss >> keyword >> p_angle;           // angle
    if (keyword != "angle") { std::cerr << "Error expected 'angle', got " << keyword << " for branch " << p_idx << std::endl; return Branch(); }

    iss >> keyword >> p_num_children;    // num_children
    if (keyword != "num_children") { std::cerr << "Error expected 'num_children', got " << keyword << " for branch " << p_idx << std::endl; return Branch(); }

    //resising vector for child indices
    p_childIndices.resize(p_num_children);
    for (int i = 0; i < p_num_children; ++i) {
        if (!(iss >> p_childIndices[i])) {
            std::cerr << "error cant read child index" << i << " for branch " << p_idx << std::endl;
            return Branch(); // default valiue retured
        }
    }
    
    // Check for any errors
    if (iss.fail() && !iss.eof()) { // eof because we are reading everything
         std::cerr << "error for branch data " << p_idx << ". details: " << iss.rdstate() << std::endl;
         return Branch(); // rreturn default
    }

// inputtings the read values into the branchs
    float angle_rad = p_angle * (M_PI / 180.0f);
    float calculated_base_x = p_cx - (0.5f * p_h * std::sin(angle_rad));
    float calculated_base_y = p_cy + (0.5f * p_h * std::cos(angle_rad));

    Branch loadedBranch(p_idx, p_parent_index, p_angle, p_h, p_w, calculated_base_x, calculated_base_y); //load the branch
    
    // restore saved age
    loadedBranch.age = p_age; 
    loadedBranch.childIndices = p_childIndices;

    // loading for data water and fertiliser

    long original_pos_sustenance = iss.tellg(); // Save position before attempting to read new fields.
    std::string keyword_sustenance_check;

    if (iss >> keyword_sustenance_check && keyword_sustenance_check == "turns_water") {
        iss >> loadedBranch.turnsWithoutWater;
        if (!(iss >> keyword_sustenance_check && keyword_sustenance_check == "turns_nutrients")) {
            std::cerr << "Parse Error: Branch " << loadedBranch.index << " expected 'turns_nutrients' after 'turns_water'. Defaulting sustenance state." << std::endl;
            iss.clear(); iss.seekg(original_pos_sustenance); // Reset to before "turns_water" attempt.
            loadedBranch.turnsWithoutWater = 0; loadedBranch.turnsWithoutNutrients = 0; loadedBranch.isAlive = true;
        } else {
            iss >> loadedBranch.turnsWithoutNutrients;
            if (!(iss >> keyword_sustenance_check && keyword_sustenance_check == "is_alive")) {
                std::cerr << "Parse Error: Branch " << loadedBranch.index << " expected 'is_alive' after 'turns_nutrients'. Defaulting sustenance state." << std::endl;
                iss.clear(); iss.seekg(original_pos_sustenance); 
                loadedBranch.turnsWithoutWater = 0; loadedBranch.turnsWithoutNutrients = 0; loadedBranch.isAlive = true;
            } else {
                iss >> loadedBranch.isAlive;
                 // If loaded as dead, ensure leaf state is consistent.
                if (!loadedBranch.isAlive) {
                    loadedBranch.hasLeaves = false;
                    loadedBranch.leafPositions.clear();
                }
            }
        }
    } else {
        // Keyword "turns_water" not found, assume old save file format for this section.
        iss.clear(); // Clear any fail bits from the attempted read.
        iss.seekg(original_pos_sustenance); // Reset stream position to before this block.
        loadedBranch.turnsWithoutWater = 0;     // Default for old saves.
        loadedBranch.turnsWithoutNutrients = 0; // Default for old saves.
        loadedBranch.isAlive = true;            // Default for old saves.
    }

    // Robustly load leaf data, compatible with older save files.
    // This block attempts to read leaf fields; if not present (old save),
    // defaults are applied based on whether the branch is alive.
    std::string potential_leaf_keyword;
    std::string keyword_leaf_check; // Used inside the block for "num_leaves" check
    long original_pos = iss.tellg(); // Remember current position

    if (iss >> potential_leaf_keyword && potential_leaf_keyword == "has_leaves") {
        iss >> loadedBranch.hasLeaves; // This hasLeaves might be overwritten if branch is dead (from sustenance block)
        if (!loadedBranch.isAlive) { // If branch is dead, ensure hasLeaves is false, regardless of file value
             loadedBranch.hasLeaves = false;
        }

        if (!(iss >> keyword_leaf_check) || keyword_leaf_check != "num_leaves") { 
            // Error or unexpected keyword after has_leaves, assume old format or corruption for leaf part
            std::cerr << "Parse Error: Branch expected 'num_leaves' after 'has_leaves' for branch " << loadedBranch.index 
                      << ". Got: " << keyword_leaf_check << ". Defaulting leaves." << std::endl;
            iss.clear(); // Clear potential error flags
            iss.seekg(original_pos); // Reset to before "has_leaves" attempt
            // If isAlive is true, default hasLeaves to true, else false. leafPositions already cleared if dead.
            loadedBranch.hasLeaves = loadedBranch.isAlive; 
            if (loadedBranch.isAlive) loadedBranch.leafPositions.clear(); // Clear for living old saves too
            else loadedBranch.leafPositions.clear(); // Already done if !isAlive, but for safety.
        } else {
            int num_leaves = 0;
            iss >> num_leaves;
            if (num_leaves < 0) { // Basic sanity check
                std::cerr << "Warning: Negative num_leaves (" << num_leaves << ") for branch " << loadedBranch.index << ". Setting to 0." << std::endl;
                num_leaves = 0; 
            }
            // Cap num_leaves to prevent excessive memory allocation if save file is malformed
            if (num_leaves > Branch::MAX_LEAVES_PER_BRANCH * 10) { // Arbitrary sanity cap, 10x max per branch
                 std::cerr << "Warning: Excessive num_leaves (" << num_leaves << ") for branch " << loadedBranch.index 
                           << ". Capping to " << Branch::MAX_LEAVES_PER_BRANCH * 10 << "." << std::endl;
                 num_leaves = Branch::MAX_LEAVES_PER_BRANCH * 10;
            }

            loadedBranch.leafPositions.resize(num_leaves); // num_leaves could be 0
            if (!loadedBranch.isAlive) { // If dead, leaves should be cleared regardless of file content
                loadedBranch.leafPositions.clear();
                num_leaves = 0; // Don't try to read leaf positions
            }

            for (int k = 0; k < num_leaves; ++k) { // Loop executes 0 times if num_leaves is 0
                if (!(iss >> loadedBranch.leafPositions[k].x >> loadedBranch.leafPositions[k].y)) {
                    std::cerr << "Error reading leaf position " << k << " for branch " << loadedBranch.index << ". Clearing remaining leaves." << std::endl;
                    loadedBranch.leafPositions.clear(); // Clear all leaves due to error
                    break; 
                }
            }
        }
    } else {
        // Keyword "has_leaves" not found, or stream ended before it. Assume old save file format for leaves.
        iss.clear(); // Clear fail bits if any
        iss.seekg(original_pos); // Reset stream position
        loadedBranch.hasLeaves = loadedBranch.isAlive; // Default for old saves (true if alive, false if dead by now)
        loadedBranch.leafPositions.clear(); // No leaf data to load for old format
    }
    
    // Final check for consistency if branch is not alive
    if (!loadedBranch.isAlive) {
        loadedBranch.hasLeaves = false;
        loadedBranch.leafPositions.clear();
    }
    
    // Check for any stream errors after trying to read all parts (including optional leaf data)
    if (iss.fail() && !iss.eof()) { // eof is fine if we read everything
         std::cerr << "Error reading branch data for index " << p_idx << " (potentially after leaves). Stream state: " << iss.rdstate() << std::endl;
        
    }

    return loadedBranch;
}
