package mooltipass.automatedTest.pageObjects;

import org.openqa.selenium.By;
import org.openqa.selenium.WebDriver;
import org.openqa.selenium.WebElement;
import org.openqa.selenium.interactions.Action;
import org.openqa.selenium.interactions.Actions;
import org.openqa.selenium.support.FindBy;
import org.openqa.selenium.support.PageFactory;
public class Icloud extends AbstractPage{
	
	public Icloud (WebDriver driver) {
		super(driver);
		PageFactory.initElements(driver, this);
	}
	
	@FindBy(xpath = "//input[@id='appleId']")
	private WebElement email;

	@FindBy(xpath = "//input[@id='pwd']")
	private WebElement password;
	
	@FindBy(xpath = "//button[@id='sign-in']//i[@class='icon icon_sign_in']")
	private WebElement submitLogin;
	
	@FindBy(xpath = "//div[@id='sc2006']")
	private WebElement menuDiv;
	
	@FindBy(xpath = "//div[@id='sc4100']")
	private WebElement logoutDiv;
	
	@FindBy(xpath = "//a[@aria-label='Close']")
	private WebElement popUp;
	
			
	public void enterEmail(String value){
		waitUntilAppears(email);
		email.sendKeys(value);
	}

	public void enterPassword(String value){

		waitUntilAppears(password);
		password.sendKeys(value);
	}
	
	public void submit(){
	submitLogin.click();
	}
	
	public void logout(){
		menuDiv.click();
		waitUntilAppears(logoutDiv);
		logoutDiv.click();
	}
	
	public boolean checkLogin(){

		waitUntilAppears(By.xpath("//a[@href='https://www.icloud.com/#contacts']"));
		return isElementPresent(By.xpath("//a[@href='https://www.icloud.com/#contacts']"));
	}
}