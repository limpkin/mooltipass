package mooltipass.automatedTest.pageObjects;

import org.openqa.selenium.By;
import org.openqa.selenium.WebDriver;
import org.openqa.selenium.WebElement;
import org.openqa.selenium.support.FindBy;
import org.openqa.selenium.support.PageFactory;

public class Minfin extends AbstractPage{

	public Minfin(WebDriver driver) {
		super(driver);
		PageFactory.initElements(driver, this);
	}
	
	@FindBy(xpath = "//input[@name='Login']")
	private WebElement email;

	@FindBy(xpath = "//input[@name='Password']")
	private WebElement password;

	@FindBy(xpath = "//div[@class='minfin-profile-widget']/div/div/div")
	private WebElement loginBtn;
	
	@FindBy(className = "mfm-auth--submit-btn")
	private WebElement submitLogin;
	
	@FindBy(xpath = "//div[@class='mfz-ud-notification mfz-ud-usermenu-btn']/a[@title='mooltipass']")
	private WebElement user;
	
	@FindBy(xpath = "//a[@href='/signout/']")
	private WebElement logoutBtn;
	
	public void goToLogin(){
		waitUntilAppears(loginBtn);		
		loginBtn.click();
	}
	
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
	
	public boolean checkLogin(){

		waitUntilAppears(By.xpath("//div[@class='mfz-ud-notification mfz-ud-usermenu-btn']/a[@title='mooltipass']"));
		return isElementPresent(By.xpath("//div[@class='mfz-ud-notification mfz-ud-usermenu-btn']/a[@title='mooltipass']"));
	}
	public boolean checkAtLoginPage(){
		return isElementPresent(By.xpath("//input[@name='Login']"));
	}
	
	public void logout(){
		user.click();
		logoutBtn.click();
		}
	
	public void closePopUpIfOpen(){
		waitUntilAppears(By.xpath("//w-div[@role='dialog']//span"));
		if(isElementPresent(By.xpath("//w-div[@role='dialog']//span"))){
				driver.findElement(By.xpath("(//w-div[@role='dialog']//span)[4]")).click();
				sleep(2000);
		}
		
	}

}
