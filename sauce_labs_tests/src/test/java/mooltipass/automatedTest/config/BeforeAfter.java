package mooltipass.automatedTest.config;

import java.io.File;
import java.io.IOException;
import java.util.concurrent.TimeUnit;

import org.openqa.selenium.OutputType;
import org.openqa.selenium.TakesScreenshot;
import org.openqa.selenium.WebDriver;

import cucumber.api.Scenario;
import cucumber.api.java.After;
import cucumber.api.java.Before;


public class BeforeAfter {

	protected static WebDriver driver;

	@Before
	public void beforeScenario() {
		getNewBrowser();

	}

	@After
	public void afterScenario(Scenario scenario) {
		if (scenario.isFailed()) {
//			try {
//				takeScreenshot(scenario.getName(), scenario.getId());
//			} catch (IOException e) {
//				e.printStackTrace();
//			}
		}
		closeBrowser();
	}



	/**
	 * Opens a new browser window and returns a new driver.
	 */
	protected static void getNewBrowser() {
		driver = WebDriverFactory.get();
		driver.manage().timeouts().implicitlyWait(5, TimeUnit.SECONDS);
	}

	/**
	 * Closes the current browser (driver).
	 */
	protected static void closeBrowser() {
		driver.quit();
		driver = null;
	}


	protected static void deleteCookies() {
		// driver.manage().deleteCookieNamed("track.mycookie");
	}

	protected void takeScreenshot(String scenarioName, String scenarioId)
			throws IOException {
		File scrFile = ((TakesScreenshot) driver)
				.getScreenshotAs(OutputType.FILE);
		// Now you can do whatever you need to do with it, for example copy
		// somewhere
		scrFile.renameTo(new File("C:\\temp",
				Long.toHexString(System.currentTimeMillis())+scenarioName + "[" + scenarioId + "]_screenshot.png"));
		
	}
}