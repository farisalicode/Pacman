void draw_pellets() 
{
    // Draw pellet's body.
    glLoadIdentity();
    glColor3f(255.0f,255.0f,255.0f);
    for (int i = 0; i < number_pellets; ++i) 
    {
        if (pellets[i].eaten == 0) 
        {
            glPushMatrix();
            glTranslatef(pellets[i].x_coordinate, pellets[i].y_coordinate, 0.0f);
            glBegin(GL_TRIANGLE_FAN);
            for (int j = 0; j <= 360; ++j) 
            {
                float angle = (j * (M_PI / 180.0));
                float x_coordinate = (pellet_radius * cos(angle));
                float y_coordinate = (pellet_radius * sin(angle));
                glVertex2f(x_coordinate, y_coordinate);
            }
            glEnd();
            glPopMatrix();
        }
    }
    return;
}

void draw_power_pellets()
{
    glColor3f(55.0f,12.0f,0.0f);
    for(int i = 0;i < number_power_pellets;++i)
    {
        if(power_pellets[i].eaten == 0)
        {
            glBegin(GL_QUADS);
            glVertex2f(power_pellets[i].x_coordinate,power_pellets[i].y_coordinate);
            glVertex2f(power_pellets[i].x_coordinate + power_pellets[i].side,power_pellets[i].y_coordinate);
            glVertex2f(power_pellets[i].x_coordinate + power_pellets[i].side, power_pellets[i].y_coordinate + power_pellets[i].side);
            glVertex2f(power_pellets[i].x_coordinate,power_pellets[i].y_coordinate + power_pellets[i].side);
            glEnd();
        }
    }
    return;
}

void draw_pacman() 
{
    sem_wait(&pacman_lock);
    // Update pacman's position.
    if(pacman.x_velocity == -velocity)
    {
        pacman.x_coordinate += pacman.x_velocity;
    }
    else if(pacman.x_velocity == +velocity)
    {
        pacman.x_coordinate += pacman.x_velocity;
    }
    if(pacman.y_velocity == -velocity)
    {
        pacman.y_coordinate += pacman.y_velocity;
    }
    else if(pacman.y_velocity == +velocity)
    {
        pacman.y_coordinate += pacman.y_velocity;
    }

   //allowing to check for power pellet

if(
        (pacman.x_coordinate < pellet2D[1][1].x_coordinate && pacman.y_coordinate < pellet2D[1][1].y_coordinate)
     || (pacman.x_coordinate < pellet2D[18][1].x_coordinate && pacman.y_coordinate > pellet2D[18][1].y_coordinate)
      || (pacman.x_coordinate > pellet2D[1][28].x_coordinate && pacman.y_coordinate < pellet2D[1][28].y_coordinate)
       || (pacman.x_coordinate > pellet2D[18][28].x_coordinate && pacman.y_coordinate > pellet2D[18][28].y_coordinate)
       ) {
             if(nowdo == 1) {
                sem_post(&allowChecking);
                nowdo = 0;
             }
         
       }
       else {
        if(nowdo == 0)
          {sem_wait(&allowChecking);
           nowdo =1;
          }
          
          
       }


   ///////////////////////////////


    glLoadIdentity();
    // Draw pacman's body.
    glColor3f(1.0f, 1.0f, 0.0f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(pacman.x_coordinate, pacman.y_coordinate);
    for (int i = 0;i <= 360;++i)
    {
        float angle = (i * (M_PI / 180.0));
        float x_coordinate = (pacman.x_coordinate + (pacman.radius * cos(angle)));
        float y_coordinate = (pacman.y_coordinate + (pacman.radius * sin(angle)));
        glVertex2f(x_coordinate,y_coordinate);
    }
    glEnd();
    // Draw pacman's mouth.
    if (draw_mouth) 
    {
        glColor3f(0.0f, 0.0f, 0.0f);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(pacman.x_coordinate, pacman.y_coordinate);
        // Moving right.
        if (pacman.x_velocity == +velocity)
        {
            for (int i = (360 - pacman.mouth_angle);i <= (360 + pacman.mouth_angle);++i) 
            {
                float angle = (i * (M_PI / 180.0));
                float x_coordinate = (pacman.x_coordinate + (pacman.radius * cos(angle)));
                float y_coordinate = (pacman.y_coordinate + (pacman.radius * sin(angle)));
                glVertex2f(x_coordinate, y_coordinate);
            }
        }
        // Moving left.
        else if (pacman.x_velocity == -velocity)
        {
            for (int i = (180 - pacman.mouth_angle);i <= (180 + pacman.mouth_angle);++i) 
            {
                float angle = (i * (M_PI / 180.0));
                float x_coordinate = (pacman.x_coordinate + (pacman.radius * cos(angle)));
                float y_coordinate = (pacman.y_coordinate + (pacman.radius * sin(angle)));
                glVertex2f(x_coordinate, y_coordinate);
            }
        }
        // Moving up.
        else if (pacman.y_velocity == -velocity)
        {
            for (int i = (270 - pacman.mouth_angle);i <= (270 + pacman.mouth_angle);++i) 
            {
                float angle = (i * (M_PI / 180.0));
                float x_coordinate = (pacman.x_coordinate + (pacman.radius * cos(angle)));
                float y_coordinate = (pacman.y_coordinate + (pacman.radius * sin(angle)));
                glVertex2f(x_coordinate, y_coordinate);
            }
        }
        // Moving down.
        else if (pacman.y_velocity == +velocity)
        {
            for (int i = (90 - pacman.mouth_angle);i <= (90 + pacman.mouth_angle);++i) 
            {
                float angle = (i * (M_PI / 180.0));
                float x_coordinate = (pacman.x_coordinate + (pacman.radius * cos(angle)));
                float y_coordinate = (pacman.y_coordinate + (pacman.radius * sin(angle)));
                glVertex2f(x_coordinate, y_coordinate);
            }
        }
        glEnd();
    }
    sem_post(&pacman_lock);
    ++mouth_frame_count;
    // Shut every 10 frames.
    if (mouth_frame_count >= 10) 
    {
        draw_mouth = !draw_mouth;
        mouth_frame_count = 0;
    }
    return;
}

void draw_ghosts()
{
    // Draw ghost's body.
    for(int i = 0;i < number_ghosts;++i)
    {
        pthread_mutex_lock(&ghosts_locks[i]);
        if(power_up == 0)
        {
            glColor3f(ghosts[i].red,ghosts[i].green,ghosts[i].blue);
        }
        else if(power_up == 1)
        {
            glColor3f(0.0f,0.0f,0.5f);
        }
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(ghosts[i].x_coordinate,ghosts[i].y_coordinate);
        for (int j = 0;j <= 360;++j)
        {
            float angle = (j * (M_PI / 180.0));
            float x_coordinate = (ghosts[i].x_coordinate + (ghosts[i].radius * cos(angle)));
            float y_coordinate = (ghosts[i].y_coordinate + (ghosts[i].radius * sin(angle)));
            glVertex2f(x_coordinate,y_coordinate);
        }
        glEnd();
        glColor3f(0.0f,0.0f,0.0f);
        glBegin(GL_TRIANGLE_FAN);
        float eye_x = (ghosts[i].x_coordinate - 3.0f);
        float eye_y = (ghosts[i].y_coordinate - 1.5f);
        glVertex2f(eye_x,eye_y);
        for (int j = 0;j <= 360;++j)
        {
            float angle = (j * (M_PI / 180.0));
            float x_coordinate = (eye_x + (2.0f * cos(angle)));
            float y_coordinate = (eye_y + (2.0f * sin(angle)));
            glVertex2f(x_coordinate,y_coordinate);
        }
        glEnd();
        glBegin(GL_TRIANGLE_FAN);
        eye_x = (ghosts[i].x_coordinate + 3.0f);
        eye_y = (ghosts[i].y_coordinate - 1.5f);
        glVertex2f(eye_x,eye_y);
        for (int j = 0;j <= 360;++j)
        {
            float angle = (j * (M_PI / 180.0));
            float x_coordinate = (eye_x + (2.0f * cos(angle)));
            float y_coordinate = (eye_y + (2.0f * sin(angle)));
            glVertex2f(x_coordinate,y_coordinate);
        }
        glEnd();
        pthread_mutex_unlock(&ghosts_locks[i]);
    }
    return;
}

void draw_walls() 
{
    // Draw boundary wall around the board.
    glColor3f(0.0f,0.0f,0.5f);
    glLineWidth(5.0f);
    glBegin(GL_LINES);
    glVertex2f(10.0f,62.0f);
    glVertex2f((board_width),62.0f);
    glVertex2f((board_width - 1.75f),62.0f);
    glVertex2f((board_width - 1.75f),(board_height));
    glVertex2f((board_width),(board_height - 3.0f));
    glVertex2f(10.0f,(board_height - 3.0f));
    glVertex2f(11.75f,(board_height));
    glVertex2f(11.75f,62.0f);
    glEnd();
    // Draw ghost house wall.
    glBegin(GL_LINES);
    glVertex2f(483.0f,356.0f);
    glVertex2f(593.0f,356.0f);
    glVertex2f(591.0f,356.0f);
    glVertex2f(591.0f,416.0f);
    glVertex2f(593.0f,416.0f);
    glVertex2f(483.0f,416.0f);
    glVertex2f(485.5f,416.0f);
    glVertex2f(485.5f,356.0f);
    glEnd();
    // Draw maze.
    glLineWidth(10.0f);
    // Segment 1.
    glBegin(GL_LINES);
    glVertex2f(308.0f,137.0f);
    glVertex2f(52.0f,137.0f);
    glVertex2f(54.75f,137.0f);
    glVertex2f(54.75f,198.0f);
    glEnd();
    // Segment 2.
    glBegin(GL_LINES);
    glVertex2f(768.75,137.0f);
    glVertex2f(1025.0f,137.0f);
    glVertex2f(1022.0f,137.0f);
    glVertex2f(1022.0f,198.0f);
    glEnd();
    // Segment 3.
    glBegin(GL_LINES);
    glVertex2f(413.0f,62.0f);
    glVertex2f(413.0f,198.0f);
    glVertex2f(415.5f,198.0f);
    glVertex2f(123.0f,198.0f);
    glEnd();
    // Segment 4.
    glBegin(GL_LINES);
    glVertex2f(663.0f,62.0f);
    glVertex2f(663.0f,198.0f);
    glVertex2f(660.0f,198.0f);
    glVertex2f(953.0f,198.0f);
    glEnd();
    // Segment 5.
    glBegin(GL_LINES);
    glVertex2f(484.0f,62.0f);
    glVertex2f(484.0f,257.0f);
    glVertex2f(486.5f,257.0f);
    glVertex2f(88.0f,257.0f);
    glEnd();
    // Segment 6.
    glBegin(GL_LINES);
    glVertex2f(592.0f,62.0f);
    glVertex2f(592.0f,257.0f);
    glVertex2f(589.5f,257.0f);
    glVertex2f(987.5f,257.0f);
    glEnd();
    // Segment 7.
    glBegin(GL_LINES);
    glVertex2f(90.5f,371.0f);
    glVertex2f(90.5f,558.0f);
    glVertex2f(90.5f,374.0f);
    glVertex2f(380.0f,374.0f);
    glVertex2f(90.5f,555.0f);
    glVertex2f(380.0f,555.0f);
    glEnd();
    // Segment 8.
    glBegin(GL_LINES);
    glVertex2f(985.0f,371.0f);
    glVertex2f(985.0f,558.0f);
    glVertex2f(985.0f,374.0f);
    glVertex2f(695.5f,374.0f);
    glVertex2f(985.0f,555.0f);
    glVertex2f(695.0f,555.0f);
    glEnd();
    // Segment 9.
    glBegin(GL_LINES);
    glVertex2f(450.0f,462.0f);
    glVertex2f(450.0f,620.0f);
    glVertex2f(450.0f,465.5f);
    glVertex2f(232.0f,465.5f);
    glVertex2f(450.0f,617.0f);
    glVertex2f(232.0f,617.0f);
    glEnd();
    // Segment 10.
    glBegin(GL_LINES);
    glVertex2f(628.0f,462.0f);
    glVertex2f(628.0f,620.0f);
    glVertex2f(628.0f,465.5f);
    glVertex2f(916.0f,465.5f);
    glVertex2f(628.0f,617.0f);
    glVertex2f(916.0f,617.0f);
    glEnd();
    // Segment 11.
    glBegin(GL_LINES);
    glVertex2f(11.75f,316.5f);
    glVertex2f(452.0f,316.5f);
    glEnd();
    // Segment 12.
    glBegin(GL_LINES);
    glVertex2f(625.5f,316.5f);
    glVertex2f(board_width,316.5f);
    glEnd();
    // Segment 13.
    glBegin(GL_LINES);
    glVertex2f(520.0f,462.0f);
    glVertex2f(520.0f,board_height);
    glEnd();
    // Segment 14.
    glBegin(GL_LINES);
    glVertex2f(557.0f,462.0f);
    glVertex2f(557.0f,board_height);
    glEnd();
    return;
}

void int_to_string(unsigned int num,char* str) 
{
    int i = 3;
    str[i] = '\0';
    do 
    {
        str[--i] = '0' + (num % 10);
        num /= 10;
    } 
    while (num != 0 && i > 0);
    if (i > 0) 
    {
        for (int j = 0; j <= 3 - i; j++) 
        { 
            str[j] = str[j + i];
        }
    }
    return;
}

void draw_state()
{
    const char* score_text = "Score: ";
    char converted_score[4];
    const char* lives_text = "           Lives: ";
    pthread_mutex_lock(&score_lock);
    int_to_string(score,converted_score);
    pthread_mutex_unlock(&score_lock);
    glLoadIdentity();
    glColor3f(1.0f, 1.0f, 1.0f);
    float posX = 40.0f;
    float posY = 40.0f;
    for (int i = 0;i < strlen(score_text);++i) 
    {
        glRasterPos2f(posX, posY);
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24,score_text[i]);
        posX += glutBitmapWidth(GLUT_BITMAP_TIMES_ROMAN_24,score_text[i]);
    }
    for (int i = 0;i < strlen(converted_score);++i) 
    {
        glRasterPos2f(posX, posY);
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24,converted_score[i]);
        posX += glutBitmapWidth(GLUT_BITMAP_TIMES_ROMAN_24,converted_score[i]);
    }
    for (int i = 0;i < strlen(lives_text);++i) 
    {
        glRasterPos2f(posX, posY);
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24,lives_text[i]);
        posX += glutBitmapWidth(GLUT_BITMAP_TIMES_ROMAN_24,lives_text[i]);
    }
    glRasterPos2f(posX, posY);
    glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24,(char)(lives + 48));
    posX += glutBitmapWidth(GLUT_BITMAP_TIMES_ROMAN_24,(char)(lives + 48));
    return;
}

void draw_game()
{
    glClear(GL_COLOR_BUFFER_BIT);
    draw_pellets();
    draw_power_pellets();
    draw_pacman();
    draw_ghosts();
    draw_walls();
    draw_state();
    glutSwapBuffers();
    ++enemyframecount;
    int allinhouse =0;


    //////////////////////////
    for(int i=0;i<4;i++){
       if(ghosts[i].inhouse == 0) {
               break;
       }

       if(i == 3) {
         allinhouse = 1;
       }
    }

////////////if all in house, release 2 immeditely

    for(int i =0 ;i<4;i++) {
        if(leavePermitAndKey[i] > 0) {
            --leavePermitAndKey[i];
            if(leavePermitAndKey[i] == 0 || allinhouse)
              {
                leavePermitAndKey[i] = -1;
                sem_post(&permit);
                sem_post(&key);
              }
        }
    }
    
    return;
}

void draw_loser() 
{
    const char* text = "GAME\n\n\nOVER\n";
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();
    glColor3f(255.0f,0.0f,0.0f);
    float posX = 150.0f;
    float posY = 150.0f;
    for (int i = 0;i < strlen(text);++i) 
    {
        if(text[i] == '\n') 
        {
            posX = 150.0f;
            posY += 20.0f;
        }
        else 
        {
            glRasterPos2f(posX, posY);
            glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24,text[i]);
            posX += glutBitmapWidth(GLUT_BITMAP_TIMES_ROMAN_24,text[i]);
        }
    }
    glutSwapBuffers();
    return;
}

void draw_winner() 
{
    const char* text = "CONGRATULATIONS\n";
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();
    glColor3f(255.0f,255.0f,0.0f);
    float posX = 70.0f;
    float posY = 150.0f;
    for (int i = 0;i < strlen(text);++i) 
    {
        if(text[i] == '\n') 
        {
            posX = 150.0f;
            posY += 20.0f;
        }
        else 
        {
            glRasterPos2f(posX, posY);
            glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24,text[i]);
            posX += glutBitmapWidth(GLUT_BITMAP_TIMES_ROMAN_24,text[i]);
        }
    }
    glutSwapBuffers();
    return;
}

void draw_menu() 
{
    const char* menu_text = "Menu\n\n\n\nPress G to start the game.\n\nPress Q to quit.\n\n";
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();
    glColor3f(1.0f, 1.0f, 1.0f);
    float posX = 50.0f;
    float posY = 50.0f;
    for (int i = 0;i < strlen(menu_text);++i) 
    {
        if(menu_text[i] == '\n') 
        {
            posX = 50.0f;
            posY += 20.0f;
        }
        else 
        {
            glRasterPos2f(posX, posY);
            glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24,menu_text[i]);
            posX += glutBitmapWidth(GLUT_BITMAP_TIMES_ROMAN_24,menu_text[i]);
        }
    }
    glutSwapBuffers();
    return;
}

void draw_ui()
{
    if(game_over == 0)
    {
        draw_menu();
    }
    else if(game_over == 1 && score < target)
    {
        draw_loser();
    }
    else if(game_over == 1 && score >= target)
    {
        draw_winner();
    }
    return;
}

