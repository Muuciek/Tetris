#include "src/States/hpp/GameState.hpp"
#include "src/GameEngine/hpp/Game.hpp"
#include "src/States/hpp/GameOverState.hpp"
#include "src/GameEngine/hpp/Components.hpp"
#include "src/DEFINE.hpp"
#include <iostream>
#include <chrono>
#include <random>
#include <map>
#include <filesystem>

namespace hgw
{
#pragma region Figure
	Figure::Figure()
	{

	}

	void Figure::Init(Figure::FigureType type, sf::Vector2f startCoords, bool classicColor = true, bool isGhostPiece = false)
	{
		setOffsetData();

		_type_ = type;
		rotationState = 0;

		switch (_type_) //set block coordinates on grid
		{
		case Figure::I:
			gridCoords[0] = sf::Vector2f(startCoords.x, startCoords.y);
			gridCoords[1] = sf::Vector2f(startCoords.x + 1, startCoords.y);
			gridCoords[2] = sf::Vector2f(startCoords.x + 2, startCoords.y);
			gridCoords[3] = sf::Vector2f(startCoords.x + 3, startCoords.y);

			pivot = &gridCoords[1]; //set pivot to rotate

			if (classicColor) //set color
			{
				setColor(sf::Color::Cyan);
			}
			break;
		case Figure::T:
			gridCoords[0] = sf::Vector2f(startCoords.x, startCoords.y + 1);
			gridCoords[1] = sf::Vector2f(startCoords.x + 1, startCoords.y + 1);
			gridCoords[2] = sf::Vector2f(startCoords.x + 2, startCoords.y + 1);
			gridCoords[3] = sf::Vector2f(startCoords.x + 1, startCoords.y);

			pivot = &gridCoords[3];

			if (classicColor)
			{
				setColor(sf::Color(128, 0, 128, 255));//purple
			}
			break;
		case Figure::O:
			gridCoords[0] = sf::Vector2f(startCoords.x, startCoords.y);
			gridCoords[1] = sf::Vector2f(startCoords.x + 1, startCoords.y);
			gridCoords[2] = sf::Vector2f(startCoords.x, startCoords.y + 1);
			gridCoords[3] = sf::Vector2f(startCoords.x + 1, startCoords.y + 1);

			pivot = nullptr; //O Figure has no pivot/no rotation

			if (classicColor)
			{
				setColor(sf::Color::Yellow);
			}
			break;
		case Figure::L:
			gridCoords[0] = sf::Vector2f(startCoords.x, startCoords.y + 1);
			gridCoords[1] = sf::Vector2f(startCoords.x + 1, startCoords.y + 1);
			gridCoords[2] = sf::Vector2f(startCoords.x + 2, startCoords.y + 1);
			gridCoords[3] = sf::Vector2f(startCoords.x + 2, startCoords.y);

			pivot = &gridCoords[1];

			if (classicColor)
			{
				setColor(sf::Color(255, 165, 0, 255)); //orange
			}
			break;
		case Figure::J:
			gridCoords[0] = sf::Vector2f(startCoords.x, startCoords.y);
			gridCoords[1] = sf::Vector2f(startCoords.x, startCoords.y + 1);
			gridCoords[2] = sf::Vector2f(startCoords.x + 1, startCoords.y + 1);
			gridCoords[3] = sf::Vector2f(startCoords.x + 2, startCoords.y + 1);

			pivot = &gridCoords[2];

			if (classicColor)
			{
				setColor(sf::Color::Blue);
			}
			break;
		case Figure::S:
			gridCoords[0] = sf::Vector2f(startCoords.x, startCoords.y + 1);
			gridCoords[1] = sf::Vector2f(startCoords.x + 1, startCoords.y);
			gridCoords[2] = sf::Vector2f(startCoords.x + 1, startCoords.y + 1);
			gridCoords[3] = sf::Vector2f(startCoords.x + 2, startCoords.y);

			pivot = &gridCoords[2];

			if (classicColor)
			{
				setColor(sf::Color::Green);
			}
			break;
		case Figure::Z:
			gridCoords[0] = sf::Vector2f(startCoords.x, startCoords.y);
			gridCoords[1] = sf::Vector2f(startCoords.x + 1, startCoords.y);
			gridCoords[2] = sf::Vector2f(startCoords.x + 1, startCoords.y + 1);
			gridCoords[3] = sf::Vector2f(startCoords.x + 2, startCoords.y + 1);

			pivot = &gridCoords[2];

			if (classicColor)
			{
				setColor(sf::Color::Red);
			}
			break;
		}

		if (!classicColor) //set random color if enabled
		{
			setColor(sf::Color(random(0, 255), random(0, 255), random(0, 255), 255));
		}

		for (int i = 0; i < 4; i++) //set block size and position on screen
		{
			blocks[i].setSize(sf::Vector2f(BLOCK_SIZE, BLOCK_SIZE));

			blocks[i].setPosition(GRID_START_POS_X + gridCoords[i].x * BLOCK_SIZE,
				GRID_START_POS_Y + gridCoords[i].y * BLOCK_SIZE);
		}

	}

	void Figure::setColor(sf::Color color)
	{
		figureColor = color;
		for (int i = 0; i < 4; i++)
		{
			blocks[i].setFillColor(color);
		}
	}

	void Figure::updateGhostCoords()
	{
		gridCoords = GameState::currentFigure.gridCoords; //set coord of ghost figure to current figure
		for (int i = 0; i < 4; i++)
		{
			blocks[i].setPosition(GameState::currentFigure.blocks[i].getPosition());
		}

		while (!willGridExceed_X(0) && !willGridExceed_Y(0) && !willBlockOverlapBlock(0, 0)) //if move possible, move down
		{
			moveFigure(sf::Vector2f(0, 1));
		}
		moveFigure(sf::Vector2f(0, -1)); //adjust height
	}

	void Figure::instaPlace()
	{
		for (int i = 0; i < 4; i++) //set current figure coords and position to ghost's
		{
			GameState::currentFigure.blocks[i].setPosition(GameState::ghostFigure.blocks[i].getPosition());
			GameState::currentFigure.gridCoords[i] = GameState::ghostFigure.gridCoords[i];
		}
	}

	void Figure::Rotate(bool clockwise, bool shouldOffest)
	{
		if (_type_ != FigureType::O) //O figure dont rotate
		{
			int oldRotationState = rotationState;

			if (clockwise) rotationState++;
			else rotationState--;

			rotationState = negMod(rotationState);

			for (int i = 0; i < 4; i++)
			{
				sf::Vector2f VrelP, Vrot, Vprim;
				VrelP = gridCoords[i] - *pivot; //calculate coords of block V relative to pivot

				if (clockwise)//calculate coords of block V after rotation relative to pivot
				{
					Vrot = sf::Vector2f(-VrelP.y, VrelP.x);
				}
				else
				{
					Vrot = sf::Vector2f(VrelP.y, -VrelP.x);
				}

				Vprim = Vrot + *pivot; //calculate coords of block V relative to grid

				gridCoords[i] = Vprim;
			}

			for (int i = 0; i < 4; i++) //set position on screen
			{
				blocks[i].setPosition(GRID_START_POS_X + gridCoords[i].x * BLOCK_SIZE, GRID_START_POS_Y + gridCoords[i].y * BLOCK_SIZE);
			}

			if (shouldOffest) //if this rotation isnt performed to revert last rotation
			{
				bool canOffset = testRotationOffset(oldRotationState, rotationState); //test if can offset and offset

				if (!canOffset)
				{
					Rotate(!clockwise, false); //if can't offset, perform revert rotation
				}
			}	
		}
	}

	void Figure::moveFigure(sf::Vector2f offset)
	{
		for (int i = 0; i < 4; i++)
		{
			gridCoords[i] += offset;
			blocks[i].move(offset.x * BLOCK_SIZE, offset.y * BLOCK_SIZE);
		}
	}

	bool Figure::willBlockOverlapBlock(float offsetX, float offsetY)
	{
		for (int i = 0; i < 4; i++) //test if there is already block on the grid
		{
			if (GameState::grid[to_uns(gridCoords[i].x + offsetX)][to_uns(gridCoords[i].y + offsetY)].first == true)
			{
				return true;
			}
		}
		return false;
	}

	bool Figure::willGridExceed_X(float offestX)
	{
		for (int i = 0; i < 4; i++)
		{
			if (gridCoords[i].x + offestX < 0 || gridCoords[i].x + offestX > 9)
			{
				return true;
			}
		}
		return false;
	}

	bool Figure::willGridExceed_Y(float offsetY)
	{
		for (int i = 0; i < 4; i++)
		{
			if (gridCoords[i].y + offsetY < 0 || gridCoords[i].y + offsetY > 19)
			{
				return true;
			}
		}
		return false;
	}

	void Figure::setOffsetData()
	{
		//data got from harddrop.com
		JLSTZ_offsetData[std::make_pair(0, 0)] = sf::Vector2f(0, 0);
		JLSTZ_offsetData[std::make_pair(0, 1)] = sf::Vector2f(0, 0);
		JLSTZ_offsetData[std::make_pair(0, 2)] = sf::Vector2f(0, 0);
		JLSTZ_offsetData[std::make_pair(0, 3)] = sf::Vector2f(0, 0);

		JLSTZ_offsetData[std::make_pair(1, 0)] = sf::Vector2f(0, 0);
		JLSTZ_offsetData[std::make_pair(1, 1)] = sf::Vector2f(1, 0);
		JLSTZ_offsetData[std::make_pair(1, 2)] = sf::Vector2f(0, 0);
		JLSTZ_offsetData[std::make_pair(1, 3)] = sf::Vector2f(-1, 0);

		JLSTZ_offsetData[std::make_pair(2, 0)] = sf::Vector2f(0, 0);
		JLSTZ_offsetData[std::make_pair(2, 1)] = sf::Vector2f(1, -1);
		JLSTZ_offsetData[std::make_pair(2, 2)] = sf::Vector2f(0, 0);
		JLSTZ_offsetData[std::make_pair(2, 3)] = sf::Vector2f(-1, -1);

		JLSTZ_offsetData[std::make_pair(3, 0)] = sf::Vector2f(0, 0);
		JLSTZ_offsetData[std::make_pair(3, 1)] = sf::Vector2f(0, 2);
		JLSTZ_offsetData[std::make_pair(3, 2)] = sf::Vector2f(0, 0);
		JLSTZ_offsetData[std::make_pair(3, 3)] = sf::Vector2f(0, 2);

		JLSTZ_offsetData[std::make_pair(4, 0)] = sf::Vector2f(0, 0);
		JLSTZ_offsetData[std::make_pair(4, 1)] = sf::Vector2f(1, 2);
		JLSTZ_offsetData[std::make_pair(4, 2)] = sf::Vector2f(0, 0);
		JLSTZ_offsetData[std::make_pair(4, 3)] = sf::Vector2f(-1, 2);


		I_offsetData[std::make_pair(0, 0)] = sf::Vector2f(0, 0);
		I_offsetData[std::make_pair(0, 1)] = sf::Vector2f(-1, 0);
		I_offsetData[std::make_pair(0, 2)] = sf::Vector2f(-1, 1);
		I_offsetData[std::make_pair(0, 3)] = sf::Vector2f(0, 1);

		I_offsetData[std::make_pair(1, 0)] = sf::Vector2f(-1, 0);
		I_offsetData[std::make_pair(1, 1)] = sf::Vector2f(0, 0);
		I_offsetData[std::make_pair(1, 2)] = sf::Vector2f(1, 1);
		I_offsetData[std::make_pair(1, 3)] = sf::Vector2f(0, 1);

		I_offsetData[std::make_pair(2, 0)] = sf::Vector2f(2, 0);
		I_offsetData[std::make_pair(2, 1)] = sf::Vector2f(0, 0);
		I_offsetData[std::make_pair(2, 2)] = sf::Vector2f(-2, 1);
		I_offsetData[std::make_pair(2, 3)] = sf::Vector2f(0, 1);

		I_offsetData[std::make_pair(3, 0)] = sf::Vector2f(-1, 0);
		I_offsetData[std::make_pair(3, 1)] = sf::Vector2f(0, 1);
		I_offsetData[std::make_pair(3, 2)] = sf::Vector2f(1, 0);
		I_offsetData[std::make_pair(3, 3)] = sf::Vector2f(0, -1);

		I_offsetData[std::make_pair(4, 0)] = sf::Vector2f(2, 0);
		I_offsetData[std::make_pair(4, 1)] = sf::Vector2f(0, -2);
		I_offsetData[std::make_pair(4, 2)] = sf::Vector2f(-2, 0);
		I_offsetData[std::make_pair(4, 3)] = sf::Vector2f(0, 2);
	}

	bool Figure::testRotationOffset(int oldRotationState, int newRotationState)
	{
		//pointer to proper offset data, to avoid copying whole map evert time (i think that is what would happend)
		std::map<std::pair<int, int>, sf::Vector2f>* currOffsetData; 

		sf::Vector2f offsetVal1, offsetVal2;
		sf::Vector2f endOffset = sf::Vector2f(0, 0);

		if (_type_ == FigureType::I) //set proper offset data
		{
			currOffsetData = &I_offsetData;
		}
		else
		{
			currOffsetData = &JLSTZ_offsetData;
		}

		for (int i = 0; i < 5; i++) //test 5 offsets
		{
			offsetVal1 = (*currOffsetData)[std::make_pair(i, oldRotationState)]; //get data for old rotation state, for current test
			offsetVal2 = (*currOffsetData)[std::make_pair(i, newRotationState)]; //get data for current rotation state, for current test
			endOffset = offsetVal1 - offsetVal2;

			if (!willGridExceed_X(endOffset.x) && !willGridExceed_Y(endOffset.y) &&
				!willBlockOverlapBlock(endOffset.x, endOffset.y)) //if move possible
			{
				moveFigure(endOffset);
				return true;
			}
		}
		return false; //move not possible
	}

	bool Figure::areCoordsGood()
	{
		for (int i = 0; i < 4; i++)
		{
			if (gridCoords[i].x < 0 || gridCoords[i].x > 9 || 
				gridCoords[i].y < 0 || gridCoords[i].y > 19)
			{
				return false;
			}
		}
		return true;
	}

	Figure::FigureType Figure::randFigureType(FigureType notToRepeatType)
	{
		int roll = random(0, 6); //"roll" figure type
		Figure::FigureType currType = static_cast<Figure::FigureType>(roll);

		if (currType == notToRepeatType) //"reroll" if last one was the same
		{
			int reroll = random(0, 6);
			currType = static_cast<Figure::FigureType>(reroll);
		}
		return currType;
	}

	Figure::FigureType Figure::randFigureType()
	{
		int roll = random(0, 6); //"roll" figure type
		Figure::FigureType currType = static_cast<Figure::FigureType>(roll);

		return currType;
	}
#pragma endregion

#pragma region GameState
	GameState::GameState(GameDataRef data)
	{
		_data = data;
	}

	void GameState::Init()
	{
		_data->sounds.LoadSoundBuffer("clear1", CLEAR1_SOUND_PATH);
		_data->sounds.LoadSoundBuffer("clear2", CLEAR2_SOUND_PATH);
		_data->sounds.LoadSoundBuffer("clear3", CLEAR3_SOUND_PATH);
		_data->sounds.LoadSoundBuffer("clear4", CLEAR4_SOUND_PATH);

		clear1.setBuffer(_data->sounds.GetSoundBuffer("clear1"));
		clear2.setBuffer(_data->sounds.GetSoundBuffer("clear2"));
		clear3.setBuffer(_data->sounds.GetSoundBuffer("clear3"));
		clear4.setBuffer(_data->sounds.GetSoundBuffer("clear4"));

		_data->music.Play(gameMusic, GAME_MUSIC_PATH, true);

		_data->graphics.LoadFont("font", FONT_PATH);
		sf::Font& font = _data->graphics.GetFont("font");

		scoreText.setFont(font);
		scoreText.setCharacterSize(50);
		scoreText.setLineSpacing(0.75f);
		scoreText.setString("Score:\n000000");

		highScoreText.setFont(font);
		highScoreText.setCharacterSize(50);
		highScoreText.setLineSpacing(0.75f);
		highScoreText.setString("Top:\n" + insertZeros(_data->saveVariables.highScore, 6));

		nextFigureText.setFont(font);
		nextFigureText.setCharacterSize(50);
		nextFigureText.setString("Next:");

		linesText.setFont(font);
		linesText.setCharacterSize(50);
		linesText.setString("Lines: 000");

		statsText.setFont(font);
		statsText.setCharacterSize(45);
		statsText.setString("Statistics:");

		levelText.setFont(font);
		levelText.setCharacterSize(50);
		levelText.setString("Level:\n  00");
			
		for (int i = 0; i < 11; i++) //set grid on screen
		{
			verticalLines[i].setSize(sf::Vector2f(1, 600));
			verticalLines[i].setPosition(sf::Vector2f(static_cast<float>(APP_WIDTH / 2 + i * 30 - 150), 100));
		}
		for (int i = 0; i < 21; i++)
		{
			horizontalLines[i].setSize(sf::Vector2f(300, 1));
			horizontalLines[i].setPosition(verticalLines[0].getPosition().x, verticalLines[0].getPosition().y + i * 30);
		}

		highScoreText.setPosition(verticalLines[10].getPosition().x + 50, 0);
		scoreText.setPosition(verticalLines[10].getPosition().x + 50, 90);
		nextFigureText.setPosition(verticalLines[10].getPosition().x + 50, scoreText.getPosition().y + 200);
		linesText.setPosition((APP_WIDTH - linesText.getGlobalBounds().width) / 2, 0);
		statsText.setPosition((APP_WIDTH - 3 * linesText.getGlobalBounds().width) / 2 - 30, 40);
		levelText.setPosition(nextFigureText.getPosition() + sf::Vector2f(0, 160));

		//set statistics figures
		stats[Figure::I].first.Init(Figure::I, sf::Vector2f(-7.5, 1), true, false);
		stats[Figure::O].first.Init(Figure::O, sf::Vector2f(-7.5, 3), true, false);
		stats[Figure::Z].first.Init(Figure::Z, sf::Vector2f(-7.5, 6), true, false);
		stats[Figure::S].first.Init(Figure::S, sf::Vector2f(-7.5, 9), true, false);
		stats[Figure::L].first.Init(Figure::L, sf::Vector2f(-7.5, 12), true, false);
		stats[Figure::J].first.Init(Figure::J, sf::Vector2f(-7.5, 15), true, false);
		stats[Figure::T].first.Init(Figure::T, sf::Vector2f(-7.5, 18), true, false);

		for (auto &fig : stats) //reset statistics data
		{
			fig.second.second = 0;
		}

		for (int typeAsInt = 0; typeAsInt < 7; typeAsInt++)
		{
			Figure::FigureType type = static_cast<Figure::FigureType>(typeAsInt);
			Figure fig = stats[type].first;

			eachStatText[type].setFont(font);
			eachStatText[type].setCharacterSize(50);	
			eachStatText[type].setString("000");

			//set statistics figures position on screen
			float xPos = horizontalLines[0].getPosition().x - 83;

			if (type == Figure::FigureType::I)
			{
				eachStatText[type].setPosition(xPos, fig.blocks[1].getPosition().y - 20);
			}
			else if (type == Figure::FigureType::L || type == Figure::FigureType::T)
			{
				eachStatText[type].setPosition(xPos, fig.blocks[3].getPosition().y);
			}
			else if (type == Figure::FigureType::J )
			{
				eachStatText[type].setPosition(xPos, fig.blocks[0].getPosition().y);
			}
			else
			{
				eachStatText[type].setPosition(xPos, fig.blocks[1].getPosition().y);
			}
		}

		//need 2 clocks for different moving speed
		dropClock.restart(); //start clock that moves blocks horizontally
		moveClock.restart(); //start clock that moves blocks vertically

		Figure::FigureType figureType = currentFigure.randFigureType();
		Figure::FigureType nextType = nextFigure.randFigureType(figureType);

		//used Init cause in this particular case constructor was causing bugs
		if (_data->saveVariables.originalColors)
		{
			currentFigure.Init(figureType, sf::Vector2f(3, 0), true, false); //create current figure with original colors
		}
		else
		{
			currentFigure.Init(figureType, sf::Vector2f(3, 0), false, false); //create current figure with random colors
		}

		ghostFigure.Init(figureType, sf::Vector2f(3, 0), true, true); //create ghost figurem

		//set ghosts figure color to current figure color with additional transparency
		ghostFigure.setColor(sf::Color(currentFigure.figureColor.r, currentFigure.figureColor.g, 
									   currentFigure.figureColor.b, currentFigure.figureColor.a - 175));
		ghostFigure.updateGhostCoords();

		if (_data->saveVariables.originalColors)
		{
			nextFigure.Init(nextType, sf::Vector2f(12, 9), true, false); //set next figure with original colors
		}
		else
		{
			nextFigure.Init(nextType, sf::Vector2f(12, 9), false, false); //set next figure with random colors
		}	
	}

	void GameState::HandleInput()
	{
		sf::Event event;

		while (_data->window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
			{
				_data->window.close();
			}
			else if (event.type == sf::Event::KeyPressed)
			{
				if (event.key.code == sf::Keyboard::Space)
				{
					currentFigure.instaPlace();

					if (currentFigure.areCoordsGood())
					{
						for (int i = 0; i < 4; i++) //add to grid
						{
							grid[to_uns(currentFigure.gridCoords[i].x)][to_uns(currentFigure.gridCoords[i].y)] = std::make_pair(true, currentFigure.blocks[i]);
						}

						stats[currentFigure._type_].second++; //update statistics
						eachStatText[currentFigure._type_].setString(insertZeros(stats[currentFigure._type_].second, 3)); //update statistics text
					}

					destroyFilledRows(); //clear lines
					setNextFigures(_data->saveVariables.originalColors); //create next figures

					for (int i = 0; i < 10; i++) //check for lose condition
					{
						if (grid[i][0].first == true)
						{
							//clear grid, cause it's static
							for (int i = 0; i < 10; i++)
							{
								for (int j = 0; j < 20; j++)
								{
									grid[i][j] = std::make_pair(false, sf::RectangleShape());
								}
							}
							_data->music.Stop(gameMusic);
							_data->machine.AddState(StateRef(new GameOverState(_data)));
						}
					}
				}
				else if (event.key.code == sf::Keyboard::X) //clockwise rotation
				{
					currentFigure.Rotate(true, true); 
					ghostFigure.updateGhostCoords();
				}
				else if (event.key.code == sf::Keyboard::Z) //counterclockwise rotation
				{
					currentFigure.Rotate(false, true);
					ghostFigure.updateGhostCoords();
				}			
			}
			else if (event.type == sf::Event::KeyReleased && event.key.code == sf::Keyboard::Down) //down key release
			{
				isDownKeyPressed = false;
			}
		}
	}

	void GameState::Update(float dt)
	{
		int dropSpeed;
		if (currLvl < 30) //calculate drop speed based on current level
		{
			dropSpeed = level[currLvl];
		}
		else
		{
			dropSpeed = 1;
		}
		
		if ((dropClock.getElapsedTime() >= sf::seconds(dropSpeed * dt) || //if need to move figure down
			(sf::Keyboard::isKeyPressed(sf::Keyboard::Down) && !isDownKeyPressed)))
		{
			if (currentFigure.willGridExceed_Y(1) || currentFigure.willBlockOverlapBlock(0, 1)) //if can't move
			{
				if (currentFigure.areCoordsGood()) //maybe can put all to this and else is game loss!!!!!!!!!!
				{
					for (int i = 0; i < 4; i++) //add to grid
					{
						grid[to_uns(currentFigure.gridCoords[i].x)][to_uns(currentFigure.gridCoords[i].y)] = std::make_pair(true, currentFigure.blocks[i]);
					}

					stats[currentFigure._type_].second++; //update statistics
					eachStatText[currentFigure._type_].setString(insertZeros(stats[currentFigure._type_].second, 3)); //update statistics text
				}		

				destroyFilledRows(); //clear lines
				setNextFigures(_data->saveVariables.originalColors); //create next figures

				//isDownKeyPressed is needed for stopping next figure from fast falling when holding down key
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
				{
					isDownKeyPressed = true;
				}
				else
				{
					isDownKeyPressed = false;
				}

				for (int i = 0; i < 10; i++) //check for lose condition
				{
					if (grid[i][0].first == true)
					{
						//clear grid
						for (int i = 0; i < 10; i++)
						{
							for (int j = 0; j < 20; j++)
							{
								grid[i][j] = std::make_pair(false, sf::RectangleShape());
							}
						}
						_data->music.Stop(gameMusic);
						_data->machine.AddState(StateRef(new GameOverState(_data)));
					}
				}
			}
			else //if can move - move
			{
				currentFigure.moveFigure(sf::Vector2f(0, 1));
			}
			dropClock.restart();
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) && moveClock.getElapsedTime() >= sf::seconds(dt * 6) &&
			!currentFigure.willGridExceed_X(1) && !currentFigure.willBlockOverlapBlock(1, 0)) //move right
		{
			currentFigure.moveFigure(sf::Vector2f(1, 0));
			ghostFigure.updateGhostCoords();

			moveClock.restart();
		}
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) && moveClock.getElapsedTime() >= sf::seconds(dt * 6) &&
			!currentFigure.willGridExceed_X(-1) && !currentFigure.willBlockOverlapBlock(-1, 0)) //move left
		{
			currentFigure.moveFigure(sf::Vector2f(-1, 0));
			ghostFigure.updateGhostCoords();

			moveClock.restart();
		}
	}

	void GameState::Draw(float dt)
	{
		_data->window.clear();

		for (int i = 0; i < 4; i++) //draw current figure, ghost figure and next figure
		{
			_data->window.draw(currentFigure.blocks[i]);
			_data->window.draw(ghostFigure.blocks[i]);
			_data->window.draw(nextFigure.blocks[i]);
		}

		for (int i = 0; i < 10; i++) //draw blocks on grid
		{
			for (int j = 0; j < 20; j++)
			{
				if (grid[i][j].first == true) //if block is on grid
				{
					_data->window.draw(grid[i][j].second);
				}
			}
		}

		if (_data->saveVariables.fullGrid) //draw full grid
		{
			for (int i = 0; i < 11; i++) 
			{
				_data->window.draw(verticalLines[i]);
			}
			for (int i = 0; i < 21; i++)
			{
				_data->window.draw(horizontalLines[i]);
			}
		}
		else //draw only grid borders
		{
			_data->window.draw(verticalLines[0]);
			_data->window.draw(verticalLines[10]);

			_data->window.draw(horizontalLines[0]);
			_data->window.draw(horizontalLines[20]);
		}
		
		
		for (auto &n : eachStatText) // draw statictic counter texts
		{
			_data->window.draw(n.second);
		}

		for (auto &n : stats) //draw statistic figures
		{
			for (int i = 0; i < 4; i++)
			{
				_data->window.draw(n.second.first.blocks[i]);
			}
		}

		_data->window.draw(scoreText);
		_data->window.draw(highScoreText);
		_data->window.draw(nextFigureText);
		_data->window.draw(linesText);
		_data->window.draw(statsText);
		_data->window.draw(levelText);

		_data->window.display();
	}

	void GameState::destroyFilledRows()
	{
		std::vector<int> filledRows = checkForRow();
		int rowsLost = 0; //how much rows got destroyed on 1 figure place
		if (filledRows.size() > 0)//if there is at least one row to destroy
		{
			for (int i = 0; i < filledRows.size(); i++) //iterates through destroyed rows Y indexes
			{
				for (int j = 0; j < 10; j++) //iterates through destroyed rows X indexes
				{
					for (int k = filledRows[i]; k > 0; k--) //move down all blocks from above deleted block
					{
						grid[j][k] = grid[j][k - 1];
						grid[j][k].second.setPosition(grid[j][k].second.getPosition().x, grid[j][k].second.getPosition().y + BLOCK_SIZE);
					}
				}
				rowsLost++;
				if (i != filledRows.size() - 1)
				{
					filledRows[i + 1] += rowsLost; //change next filled row index, cause all block above target were moved down
				}
			}

			totalRowsCleaned += rowsLost;

			switch (rowsLost) //update score and play proper sound
			{
			case 1:
				score += 40 * (currLvl + 1);
				_data->sounds.Play(clear1);
				break;
			case 2:
				score += 100 * (currLvl + 1);
				_data->sounds.Play(clear2);
				break;
			case 3:
				score += 300 * (currLvl + 1);
				_data->sounds.Play(clear3);
				break;
			case 4:
				score += 1200 * (currLvl + 1);
				_data->sounds.Play(clear4);
			}

			//GameState::rowsCleanedtext.setString("Score: " + score);// causes nice bug
			GameState::scoreText.setString("Score:\n" + insertZeros(score, 6));

			if (score > _data->saveVariables.highScore) //if current score is higher than highscore
			{
				updateGameData(GameData::variableNames::highScore, score, _data); //set highscore to current score
				highScoreText.setString("Top:\n" + insertZeros(score, 6)); //update highscore text
			}

			if (totalRowsCleaned % 10 == 0) //increase level if needed
			{
				currLvl++;
				levelText.setString("Level:\n  " + insertZeros(currLvl, 2));
			}

			linesText.setString("Lines: " + insertZeros(totalRowsCleaned, 3)); //update lines counter text
		}		
	}

	std::vector<int> GameState::checkForRow() //returns indexes of filled rows
	{
		std::vector<int> filledRows;
		bool filled = true;

		for (int i = 19; i >= 0; i--) //iterates through grid Ys
		{
			filled = true;
			for (int j = 0; j < 10; j++) //iterates through grid Xs
			{
				if (grid[j][i].first == false) //if row is not filled -> skip i iteration
				{
					filled = false;
					break;
				}
			}
			if (filled)
			{
				filledRows.push_back(i);
			}
		}
		return filledRows;
	}

	int random(int min, int max)
	{
		unsigned seed = static_cast<unsigned>(std::chrono::system_clock::now().time_since_epoch().count());
		std::mt19937 gen(seed);
		std::uniform_int<int> distrib(min, max);
		return distrib(gen);
	}

	int negMod(int val) //modulo that works for negative numbers as well
	{
		if (val < 0)
		{
			return val % 4 + 4;
		}
		return val % 4;
	}

	void GameState::setNextFigures(bool classicColor) //create and set next figures (ghost, current and next next)
	{
		currentFigure.Init(nextFigure._type_, sf::Vector2f(3, 0), classicColor, false);

		ghostFigure.Init(nextFigure._type_, sf::Vector2f(3, 0), classicColor, true);
		ghostFigure.setColor(sf::Color(currentFigure.figureColor.r, currentFigure.figureColor.g,
			currentFigure.figureColor.b, currentFigure.figureColor.a - 175));
		ghostFigure.updateGhostCoords();
		
		Figure::FigureType nextType = nextFigure.randFigureType(currentFigure._type_);
		nextFigure.Init(nextType, sf::Vector2f(12, 9), true, false);
	}

	void GameState::updateGameData(GameData::variableNames variableName, int variableValue, GameDataRef _data)
	{
		std::fstream newFile;
		std::string variableName_str; //variable name to search in file

		switch (variableName) //set proper variable name
		{
		case hgw::GameData::highScore:
			_data->saveVariables.highScore = variableValue;
			variableName_str = "highscore";
			break;

		case hgw::GameData::fullGrid:
			_data->saveVariables.fullGrid = variableValue;
			variableName_str = "fullgrid";
			break;

		case hgw::GameData::originalColors:
			variableName_str = "originalcolors";
			_data->saveVariables.originalColors = variableValue;
		}

		newFile.open("tempData.dat", std::fstream::out); //create temp file to save new data
		dataFile.open("data.dat", std::fstream::out | std::fstream::in); //open current data file

		if (dataFile.good() && newFile.good()) //not fail
		{
			newFile << variableName_str + " " + std::to_string(variableValue) << "\n"; //write updated variable to new file

			std::string line;
			while (std::getline(dataFile, line)) //copy all lines from current data file to new file except updated variable
			{
				if (line.find(variableName_str) == std::string::npos)
				{
					newFile << line << "\n";
				}
			}

			dataFile.close();
			newFile.close();

			std::experimental::filesystem::remove("data.dat"); //remove current data file
			std::experimental::filesystem::rename("tempData.dat", "data.dat"); //rename temp data file to current data file
		}

		dataFile.close();
		newFile.close();		
	}

	int GameState::getGameDataFromFile(GameData::variableNames variableName)
	{
		std::string toFind; //data to find in file
		toFind = varNameAsStr(variableName); //convert enum type 'GameData::variableNames' to proper string

		dataFile.open("data.dat", std::fstream::out | std::fstream::in);

		if (dataFile.good())
		{
			std::string line;
			while (std::getline(dataFile, line))
			{
				if (line.find(toFind) != std::string::npos) //if this line contatins variable we want to find
				{
					std::string score = line.erase(0, toFind.size()); //convert file line to value
					//eg. highscore 20 -> 20
					dataFile.close();
					return std::stoi(score); //return score as int
				}
			}
		}
		dataFile.close();

		return 0;
	}

	std::string GameState::insertZeros(int value, int digits) //insert zeros to number string (eg. insertZeros(5, 3) -> 005)
	{
		std::string scoreString = std::to_string(value);
		int zerosToInsert = digits - scoreString.size();
		scoreString.insert(0, zerosToInsert, '0');

		return scoreString;
	}

	std::string GameState::varNameAsStr(GameData::variableNames varName) //convert enum type 'GameData::variableNames' to proper string
	{
		switch (varName) 
		{
		case GameData::highScore:
			return "highscore";
		case GameData::fullGrid:
			return "fullgrid";
		case GameData::originalColors:;
			return "originalcolors";
		}
	}

	GameData::variableNames GameState::asVarName(std::string varNameAsStr) //convert string to proper enum type 'GameData::variableNames' variable
	{
		if (varNameAsStr == "highscore")
		{
			return GameData::highScore;
		}
		else if (varNameAsStr == "fullgrid")
		{
			return GameData::fullGrid;
		}
		else if (varNameAsStr == "originalcolors")
		{
			return GameData::originalColors;
		}
	}

#pragma endregion
	
	//static variables
	std::map<std::pair<int, int>, sf::Vector2f> Figure::I_offsetData;
	std::map<std::pair<int, int>, sf::Vector2f> Figure::JLSTZ_offsetData;

	std::array<std::array<std::pair<bool, sf::RectangleShape>, 20>, 10> GameState::grid;

	Figure GameState::currentFigure;
	Figure GameState::ghostFigure;
	Figure GameState::nextFigure;

	std::fstream GameState::dataFile;
}