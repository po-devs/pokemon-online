/*
 * Copyright (C) 2008 Remko Troncon
 */

#include "SparkleAutoUpdater.h"

#include <Cocoa/Cocoa.h>
#include <Sparkle/Sparkle.h>

class SparkleAutoUpdater::Private
{
	public:
		SUUpdater* updater;
};

SparkleAutoUpdater::SparkleAutoUpdater()
{
	d = new Private;

	d->updater = [SUUpdater sharedUpdater];
	[d->updater retain];
}

SparkleAutoUpdater::~SparkleAutoUpdater()
{
	[d->updater release];
	delete d;
}

void SparkleAutoUpdater::checkForUpdates()
{
	[d->updater checkForUpdatesInBackground];
}
