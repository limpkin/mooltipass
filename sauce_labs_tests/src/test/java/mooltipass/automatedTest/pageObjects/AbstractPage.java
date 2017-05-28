package mooltipass.automatedTest.pageObjects;

import org.openqa.selenium.By;
import org.openqa.selenium.NoSuchElementException;
import org.openqa.selenium.WebDriver;
import org.openqa.selenium.WebElement;
import org.openqa.selenium.support.ui.WebDriverWait;


public class AbstractPage {
	protected WebDriver driver;
	protected WebDriverWait wait;

	public AbstractPage(WebDriver driver) {
		this.driver = driver;
		wait = new WebDriverWait(driver, 30);
	}

	protected static void sleep(int ms) {
		try {
			Thread.sleep(ms);
		} catch (InterruptedException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}

	protected boolean isElementPresent(By by) {
		try {
			driver.findElement(by);
			return true;
		} catch (NoSuchElementException e) {
			return false;
		}
	}

	protected void enter(WebElement element, String keys) {
		element.clear();
		element.sendKeys(keys);	
	}
	
	
	
	protected void click(WebElement element) {
		element.click();
	}
}


