characterClassesArray = {
	Citizen = {
		class = "Citizen",
		attributes = {
			strength = 25,
			speed = 25,
			magic = 25,
			accuracy = 25,
			stealth = 25,
			constitution = 25,
		},
		titles = {
			"Serviceman",
			"Color Bearer",
			"King's Militia",
			"Loyalist",
			"Crown's Servant",
			"Crown's Fighter",
			"Warrior",
			"Defender",
			"Noble",
			"Royal Courtsman",
			"Brigade Elmore",
			"Righteous Band",
			"League of Aeneas",
			"Esias's Servant",
			"Exalted Guardian",
			"Hero",
			"Court Knight",
			"Guard Commander",
			"Supreme General",
			"The Mighty"
		},
		advancement = {
			strength = 2,
			speed = 2,
			magic = 2,
			accuracy = 2,
			stealth = 2,
			constitution = 2,
		},
		description = "This class has slightly above average attributes in all areas but lacks any special abilities.  He is especially suited to beginning gamers.\rArmor Types: All\rWeapon Types: All\rMagik System: Arcane\r",
		portrait_name = "UI/CREATEC/CHAR00",
		init_inventory = function()
		end
	},
	Knight = {
		class = "Knight",
		attributes = {
			strength = 35,
			speed = 15,
			magic = 10,
			accuracy = 20,
			stealth = 15,
			constitution = 35,
		},
		titles =  {
			"Page",
			"Trainee",
			"Squire",
			"Swordsman",
			"Enlisted Servant",
			"Tactical Student",
			"Swordsmaster",
			"Court Guard",
			"Armsmaster",
			"Knight",
			"Sect of Elmore",
			"Royal Captain",
			"Royal General",
			"King's Guardian",
			"Vanquisher",
			"Sword of Titas",
			"Glory of Aelia",
			"Horn of Domitian",
			"Title of Omega",
			"Dragonslayer"
		},
		advancement = {
			strength = 3,
			speed = 2,
			magic = 1,
			accuracy = 2,
			stealth = 1,
			constitution = 3,
		},
		description = "Characters of the Knight class are specialized in melee combat.  Knights receive bonuses to damage done with melee weapons.\r\rArmor Types: All\rWeapon Types: All\rMagik System: Arcane\r",
		portrait_name = "UI/CREATEC/CHAR01",
		init_inventory = function()
		end
	},
	Mage = {
		class = "Mage",
		attributes = {
			strength = 10,
			speed = 25,
			magic = 35,
			accuracy = 25,
			stealth = 20,
			constitution = 15,
		},
		titles = {
			"Student",
			"Apprentice",
			"Spellcaster",
			"Mage",
			"Conjurer",
			"Enchanter",
			"Wizard",
			"Lore Master",
			"Dark Spirit",
			"Black Art Scribe",
			"Black Art Master",
			"Magi of the Fire",
			"Magi of the Star",
			"Sorcerer",
			"Order of Nu'ak",
			"Order of Tul",
			"Order of Ahnul",
			"Order of Baal",
			"Prophet of Baal",
			"Ancient One"
		},
		advancement = {
			strength = 1,
			speed = 2,
			magic = 3,
			accuracy = 3,
			stealth = 2,
			constitution = 1,
		},
		description = "Mages wield powerful magiks but are physically weak.  Mages rely solely on magik to make them formidable characters.\r\rArmor Types: Leather\rWeapon Types: Staff/Dagger\rMagik System: Mage\r",
		portrait_name = "UI/CREATEC/CHAR02",
		init_inventory = function()
		end
	},
	Warlock = {
		class = "Warlock",
		attributes = {
			strength = 25,
			speed = 20,
			magic = 30,
			accuracy = 20,
			stealth = 15,
			constitution = 20,
		},
		titles = {
			"Disciple",
			"Apprentice",
			"Dark Artesian",
			"Warlock",
			"Mighty Sword",
			"Vanquisher",
			"Legion",
			"Destroyer",
			"Mystic Conquerer",
			"1st Circle Order",
			"2nd Circle Order",
			"3rd Circle Order",
			"Summoner",
			"Seeker",
			"Destructor",
			"Desolator",
			"Dragon Spirit",
			"Ruiner",
			"Death's Follower",
			"Death Incarnate"
		},
		advancement = {
			strength = 2,
			speed = 2,
			magic = 2,
			accuracy = 2,
			stealth = 2,
			constitution = 2,
		},
		description = "Warlocks are mages trained especially for combat.  They are not as powerful in the magiks as mages, but have devoted time to studying melee combat techniques.\rArmor Types: Up to chain\rWeapon Types: No crossbow\rMagik System: Mage\r",
		portrait_name = "UI/CREATEC/CHAR03",
		init_inventory = function()
		end
	},
	Priest = {
		class = "Priest",
		attributes = {
			strength = 15,
			speed = 20,
			magic = 35,
			accuracy = 15,
			stealth = 20,
			constitution = 25,
		},
		titles = {
			"Flock Tender",
			"Student",
			"Scribe",
			"Teacher",
			"Evangelist",
			"Cleric",
			"Elder",
			"The Annointed",
			"Parable Master",
			"Master of Canon",
			"Bishop",
			"Minor Prophet",
			"Cross Bearer",
			"Disciple",
			"Sacred Branch",
			"Witness",
			"Apostle",
			"Saint",
			"Herald of Glory",
			"Illuminated One"
		},
		advancement = {
			strength = 2,
			speed = 1,
			magic = 3,
			accuracy = 2,
			stealth = 2,
			constitution = 2,
		},
		description = "Characters devoted to becoming Priests gain powers of healing and protection.  They can also fight reasonably well.\r\rArmor Types: Up to Chain\rWeapon Types: Mace/Staff\rMagik System: Priest\r",
		portrait_name = "UI/CREATEC/CHAR04",
		init_inventory = function()
		end
	},
	Rogue = {
		class = "Rogue",
		attributes = {
			strength = 15,
			speed = 30,
			magic = 15,
			accuracy = 25,
			stealth = 30,
			constitution = 15,
		},
		titles = {
			"Sneak",
			"Shadow",
			"Pickpocket",
			"Rogue",
			"Trickster",
			"Contriver",
			"Thief",
			"Knave",
			"Skilled Con",
			"Scoundrel",
			"Nighthawk",
			"Teacher",
			"1st Sect Member",
			"2nd Sect Member",
			"3rd Sect Member",
			"Master of the Art",
			"Stealth Master",
			"The Upright",
			"Master Thief",
			"Legendary Delver"
		},
		advancement = {
			strength = 2,
			speed = 2,
			magic = 1,
			accuracy = 2,
			stealth = 3,
			constitution = 2,
		},
		description = "Rogues specialize in sneaking around and picking locks and pockets.  They are very quick but rely on stealth instead of combat.\r\rArmor Types: Leather\rWeapon Types: No longsword\rMagik System: Arcane\r",
		portrait_name = "UI/CREATEC/CHAR05",
		init_inventory = function()
		end
	},
	Archer = {
		class = "Archer",
		attributes = {
			strength = 20,
			speed = 20,
			magic = 15,
			accuracy = 35,
			stealth = 20,
			constitution = 20,
		},
		titles = {
			"Fletcher",
			"Journeyman",
			"Page",
			"Squire",
			"Marksman",
			"Targetman",
			"Noble",
			"Sharpshooter",
			"Warrior",
			"Acurate",
			"Archer",
			"Forest Arrow",
			"CraftMaster",
			"Ancient Arrow",
			"Huntsman (1rd)",
			"Huntsman (2nd)",
			"Huntsman (3rd)",
			"Divine Missile",
			"Unerring One",
			"The Mighty"
		},
		advancement = {
			strength = 2,
			speed = 2,
			magic = 2,
			accuracy = 3,
			stealth = 1,
			constitution = 2,
		},
		description = "Archers are the masters of missile weapons.\r\r\r\r\rArmor Types: Up to chain\rWeapon Types: Any\rMagik System: Arcane\r",
		portrait_name = "UI/CREATEC/CHAR06",
		init_inventory = function()
		end
	},
	Sailor = {
		class = "Sailor",
		attributes = {
			strength = 25,
			speed = 25,
			magic = 10,
			accuracy = 20,
			stealth = 15,
			constitution = 35,
		},
		titles = {
			"Landlubber",
			"Trainee",
			"Galley Grunt",
			"Seaman",
			"Mate",
			"First Mate",
			"Sailor",
			"Veteran",
			"Captain",
			"Degree Hellespont",
			"Order of Salamis",
			"Golden Horn",
			"Seamaster",
			"Sextant Lord",
			"Admiral",
			"Water Magician",
			"Fury",
			"Order of Ulysses",
			"Prince of Havens",
			"Poseidon's Guide"
		},
		advancement = {
			strength = 2,
			speed = 2,
			magic = 1,
			accuracy = 2,
			stealth = 2,
			constitution = 3,
		},
		description = "Sailors are quick on their feet and are brutal fighters, but are weak in the magiks.\r\r\r\rArmor Types: Up to chain\rWeapon Types: Any\rMagik System: Arcane\r",
		portrait_name = "UI/CREATEC/CHAR07",
		init_inventory = function()
		end
	},
	Paladin = {
		class = "Paladin",
		attributes = {
			strength = 30,
			speed = 20,
			magic = 25,
			accuracy = 15,
			stealth = 10,
			constitution = 30,
		},
		titles = {
			"Page",
			"Squire",
			"Servant",
			"Good Knight",
			"The Annointed",
			"Blood Knight",
			"Warrior",
			"Son of Thunder",
			"Blessed Sword",
			"Crusader",
			"Champion",
			"Disciple",
			"Vanquisher",
			"Illuminated",
			"Cross Bearer",
			"Saint of Steel",
			"Soldier of God",
			"Avenger of God",
			"Wrath of God",
			"Hand of God"
		},
		advancement = {
			strength = 3,
			speed = 2,
			magic = 2,
			accuracy = 2,
			stealth = 1,
			constitution = 2,
		},
		description = "Paladins are formidable fighters blessed with the holy word.\r\r\r\rArmor Types: Any\rWeapon Types: No bow\rMagik System: Priest\r",
		portrait_name = "UI/CREATEC/CHAR08",
		init_inventory = function()
		end
	},
	Mercenary = {
		class = "Mercenary",
		attributes = {
			strength = 30,
			speed = 20,
			magic = 10,
			accuracy = 25,
			stealth = 20,
			constitution = 25,
		},
		titles = {
			"Brawler",
			"Fighter",
			"Hireling",
			"Ransacker",
			"Wolf of War",
			"Soldier Fortune",
			"Destroyer",
			"Valorian Vitorix",
			"Legionaire",
			"Asassin",
			"Axe of Alesia",
			"Avenger",
			"Theo's Sword",
			"Siege Master",
			"Captain",
			"Son of Sparta",
			"The Janissary",
			"Hero",
			"ArmsMaster",
			"Casca"
		},
		advancement = {
			strength = 2,
			speed = 2,
			magic = 2,
			accuracy = 2,
			stealth = 2,
			constitution = 2,
		},
		description = "Mercenaries have had a hard life, and have learned to fight and steal to survive.\r\r\r\rArmor Types: Any\rWeapon Types: No mace\rMagik System: Arcane\r",
		portrait_name = "UI/CREATEC/CHAR09",
		init_inventory = function()
		end
	},
	Magician = {
		class = "Magician",
		attributes = {
			strength = 15,
			speed = 30,
			magic = 30,
			accuracy = 20,
			stealth = 20,
			constitution = 15,
		},
		titles = {
			"Pupil",
			"Student",
			"Trickster",
			"Showman",
			"Charlatan",
			"Magician",
			"Deceiver",
			"Charmer",
			"Mystic",
			"Illusionist",
			"RuneMaster",
			"Seer",
			"Sage",
			"Dark Oracle",
			"Student of Rit",
			"Keeper of Rit",
			"Teacher of Rit",
			"Tower Artesian",
			"Master Sage",
			"Dark Prophet"
		},
		advancement = {
			strength = 1,
			speed = 3,
			magic = 3,
			accuracy = 2,
			stealth = 1,
			constitution = 2,
		},
		description = "Magicians are trickster-mages, and will often times pick your pockets while they mesmerise you with magik\r\rArmor Types: Leather\rWeapon Types: Sword/Dagger/Staff\rMagik System: Mage\r",
		portrait_name = "UI/CREATEC/CHAR10",
		init_inventory = function()
		end
	},
};

characterClasses = {}

characterClasses.findPrevious = function(class)
	local previous = nil;
	for key, value in pairs(characterClassesArray) do
		if (characterClassesArray[key].class == class) then
			break;
		end
		previous = characterClassesArray[key].class;
	end	
	if (previous == nil) then
		-- First on the list, find the last one
		for key, value in pairs(characterClassesArray) do
			previous = characterClassesArray[key].class;
		end	
	end
	return previous;
end

characterClasses.findNext = function(class)
	local previous = nil;
	local now = nil;
	local first = nil;
	for key, value in pairs(characterClassesArray) do
		if (first == nil) then
			first = characterClassesArray[key].class;
		end
		now = characterClassesArray[key].class;
		if (previous == class) then
			return now;
		end
		previous = characterClassesArray[key].class;
	end
	return first;	
end

characterClasses.find = function(className)
	return characterClassesArray[className];
end

return chraacterClasses;
